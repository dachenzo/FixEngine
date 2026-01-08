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

        void add_soh();

        void grow(std::size_t extra);

        void insert_body_length();

        void insert_checksum();

        std::size_t get_position() const;

        std::size_t get_buffer_size() const;

        std::size_t compute_body_length() const noexcept;

        std::size_t compute_checksum() const noexcept;

        std::string_view get_buffer_view();

        private:
        std::byte* buffer = nullptr;
        std::size_t buffer_size = 64 * 1024;// 64 KB
        std::size_t position  = 0;
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

        std::string_view resend_request(int begin_seq_no, int end_seq_no) {
            scratch_.reset();
            stamp_header_("2");
            scratch_.add_field(7,  std::to_string(begin_seq_no));
            scratch_.add_field(16, std::to_string(end_seq_no));
            stamp_trailer_();
            return scratch_.get_buffer_view();
        }
        std::string_view sequence_reset(int newSeqNo, bool gapfill) {
            scratch_.reset();
            stamp_header_("4");
            if (gapfill) {
                scratch_.add_field(123, "Y");   // GapFillFlag
            }
            scratch_.add_field(36, std::to_string(newSeqNo)); // NewSeqNo
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



        private:
        FactoryScratch scratch_;    
        SessionParameters& params_;
        SeqProvider& seq_provider_;
        TClock& clock_;

        void stamp_header_(std::string type) {
            scratch_.add_field(8, params_.fix_version);
            scratch_.add_body_length_placeholder();
            scratch_.add_field(35, type);
            scratch_.add_field(34, std::to_string(seq_provider_.next_out()));
            scratch_.add_field(49, params_.sender_comp_id);
            scratch_.add_field(56, params_.target_comp_id);
            scratch_.add_field(52, clock_.now_fix());
        }

        void stamp_trailer_()  {
            scratch_.insert_body_length();
            scratch_.insert_checksum();
        }
    };

}
