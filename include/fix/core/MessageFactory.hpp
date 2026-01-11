#pragma once
#include <optional>
#include <string>
#include <cstdint>
#include <charconv>
#include <cstring>
#include <string_view>
#include <algorithm>
#include <fix/core/definitions.hpp>
#include <fix/core/Clock.hpp>
#include <fix/core/SeqProvider.hpp>
#include <fix/core/MessageStore.hpp>
#include <fix/core/Message.hpp>

namespace Fix {

    struct FactoryScratch {
        static constexpr const std::size_t min_grow_size = 1024;
        
        FactoryScratch();
        FactoryScratch(FactoryScratch const&) = delete;
        FactoryScratch& operator=(FactoryScratch const&) = delete;
        FactoryScratch(FactoryScratch&&) = delete;
        FactoryScratch& operator=(FactoryScratch&&) = delete;

        ~FactoryScratch();

        void reset() ;

        void add_int(int64_t value);

        void add_string(std::string_view sv);

        void add_body_length_placeholder();

        void add_equal_sign();

        void add_char(unsigned char c);

        void add_field(int64_t tag, std::string_view value);

        void add_field(int64_t tag, int64_t value);

        void add_soh();

        void grow(std::size_t extra);

        void insert_body_length();

        void insert_checksum();

        void edit_window(std::size_t offset, std::size_t length, std::string_view new_data);

        std::size_t get_position() const;

        std::size_t get_buffer_size() const;

        std::size_t compute_body_length() const noexcept;

        std::size_t compute_checksum() const noexcept;

        std::string_view get_buffer_view();

        private:
        std::byte* buffer = nullptr;
        std::size_t buffer_size = 64 * 1024;// 64 KB
        std::size_t position = 0;
        std::size_t body_length_offset = 0;
        std::size_t body_length_digit_size = 10;// realistically will not exceed this
        std::size_t checksum_offset = 0;
        char int_buff[22]; // enough for 64-bit int

 
    };

    template<ClockLike TClock>
    struct MessageFactory {
        MessageFactory(SessionParameters& params, SeqProvider& seq_provider, TClock& clock): scratch_{}, params_{params}, seq_provider_{seq_provider}, clock_{clock} {
        }



        std::string_view logon(int heartbeat_override, bool echo_reset) {
            scratch_.reset();
            stamp_header_("A");
            scratch_.add_field(98, params_.encrypt_method_str);
            scratch_.add_field(141, (echo_reset ? "Y" : "N"));
            scratch_.add_field(108, params_.heart_beat_str);
            stamp_trailer_();
            return scratch_.get_buffer_view();
        }

        std::string_view heart_beat(std::optional<std::string> test_req_id = std::nullopt) {
            scratch_.reset();
            stamp_header_("0");
            if (test_req_id.has_value()) {
                scratch_.add_field(112, test_req_id.value());
            }
            stamp_trailer_();
            return scratch_.get_buffer_view();
        }
        std::string_view test_request(std::string id) {
            scratch_.reset();
            stamp_header_("1");
            scratch_.add_field(112, id);
            stamp_trailer_();
            return scratch_.get_buffer_view();
        }

        std::string_view resend_request(uint64_t begin_seq_no, uint64_t end_seq_no) {
            scratch_.reset();
            stamp_header_("2");
            scratch_.add_field(7,  static_cast<int64_t>(begin_seq_no));
            scratch_.add_field(16, static_cast<int64_t>(end_seq_no));
            stamp_trailer_();
            return scratch_.get_buffer_view();
        }
        std::string_view sequence_reset(uint64_t newSeqNo, bool gapfill) {
            scratch_.reset();
            stamp_header_("4");
            if (gapfill) {
                scratch_.add_field(123, "Y");   // GapFillFlag
            }
            scratch_.add_field(36, static_cast<int64_t>(newSeqNo)); // NewSeqNo
            // If this is going out in a resend stream: add 43=Y and 122=...
            stamp_trailer_();
            return scratch_.get_buffer_view();
        }
        
        std::string_view logout(std::string text = {}) {
            scratch_.reset();
            stamp_header_("5");
            if (!text.empty()) {
                scratch_.add_field(58, text);
            }
            stamp_trailer_();
            return scratch_.get_buffer_view();
        }

        std::string_view reject(std::size_t ref_seq_num, uint32_t reason, std::size_t tag, std::string text ) {
            scratch_.reset();
            stamp_header_("3");
            scratch_.add_field(45, std::to_string(ref_seq_num));
            if (tag != 0) {
                scratch_.add_field(371, std::to_string(tag));
            }
            scratch_.add_field(373, std::to_string(reason));
            if (!text.empty()) {
                scratch_.add_field(58, text);
            }
            stamp_trailer_();
            return scratch_.get_buffer_view();
        }

        std::string_view regenerate_message(std::string_view original_wire, MsgIndex& msg_index) {
            scratch_.reset();
            scratch_.add_field(8, params_.fix_version);
            scratch_.add_body_length_placeholder();
            auto position_after_body_length = scratch_.get_position(); 
            auto body = get_msg_body(original_wire);
            auto body_offset = get_msg_body_offset(original_wire);
            scratch_.add_string(body);

            if ( msg_index.off_43 == -1) {
                scratch_.add_field(43, "Y");
            } else {
                scratch_.edit_window(position_after_body_length + msg_index.off_43 - body_offset, msg_index.len_43, "Y");
            }

            if (msg_index.off_122 == -1) {
                scratch_.add_field(122, original_wire.substr(msg_index.off_52, msg_index.len_52));
            } else {
                scratch_.edit_window(position_after_body_length + msg_index.off_122 - body_offset, msg_index.len_122, original_wire.substr(msg_index.off_52, msg_index.len_52));
            }

            // 52 has to be present because we are regenerating 
            scratch_.edit_window(position_after_body_length + msg_index.off_52 - body_offset, msg_index.len_52, clock_.now_fix());

            stamp_trailer_();
            return scratch_.get_buffer_view();

            
        }



        private:
        FactoryScratch scratch_;    
        SessionParameters& params_;
        SeqProvider& seq_provider_;
        TClock& clock_;

        void stamp_header_(std::string type) {
            scratch_.add_field(8, params_.fix_version);
            scratch_.add_body_length_placeholder();
            scratch_.add_field(35, type);
            scratch_.add_field(34, static_cast<int64_t>(seq_provider_.next_out()));
            scratch_.add_field(49, params_.sender_comp_id);
            scratch_.add_field(56, params_.target_comp_id);
            scratch_.add_field(52, clock_.now_fix());
        }

        void stamp_trailer_()  {
            scratch_.insert_body_length();
            scratch_.insert_checksum();
        }

        std::string_view get_msg_body(std::string_view wire) {
            // requires that wire is a complete FIX message
            // in format 8=...|9=...|35=...|...|10=...
            // returns the portion between body length and checksum
            auto begin = wire.find("9=");
            begin = wire.find('\x01', begin);
            begin += 1; // move past SOH
            auto end = wire.rfind("10=");
            return wire.substr(begin, end - begin);
        }

        std::size_t get_msg_body_offset(std::string_view wire) {
            auto begin = wire.find("9=");
            begin = wire.find('\x01', begin);
            begin += 1; // move past SOH
            return begin;
        }

        
    };

}
