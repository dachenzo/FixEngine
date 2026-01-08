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

    struct MessageFactory {
        MessageFactory(SessionParameters& params, SeqProvider& seq_provider, Clock& clock);


        std::string_view logon(int heartbeat_override, bool echo_reset);
        Fix::ValidMessage heart_beat(std::optional<std::string> test_req_id = std::nullopt);
        Fix::ValidMessage test_request(std::string id);
        Fix::ValidMessage resend_request(int begin_seq_no, int end_seq_no);
        Fix::ValidMessage sequence_reset(int newSeqNo, bool gapfill);
        Fix::ValidMessage logout(std::string text = {});



        private:
        FactoryScratch scratch_;    
        SessionParameters& params_;
        SeqProvider& seq_provider_;
        Clock& clock_;

        void stamp_header_(std::string type);
        void stamp_trailer_();
        std::uint64_t compute_body_length_(Fix::ValidMessage& msg);
        inline std::uint32_t sum_bytes_(std::string_view v);
        inline std::uint32_t sum_tag_ascii_(int tag);
        std::uint8_t compute_check_sum_
        (Fix::ValidMessage& msg);
        constexpr int tag_count_(int tag) const noexcept;

        

    };

}
