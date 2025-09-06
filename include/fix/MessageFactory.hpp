#pragma once
#include <optional>
#include <string>
#include <cstdint>
#include <string_view>
#include <fix/definitions.hpp>
#include <fix/Clock.hpp>
#include <fix/SeqProvider.hpp>
#include <fix/Message.hpp>

namespace Fix {

    struct MessageFactory {
        MessageFactory(SessionParameters& params, SeqProvider& seq_provider, Clock& clock);


        Fix::Message logon(int heartbeat_override, bool echo_reset);
        Fix::Message heart_beat(std::optional<std::string> test_req_id = std::nullopt);
        Fix::Message test_request(std::string id);
        Fix::Message resend_request(int begin_seq_no, int end_seq_no);
        Fix::Message sequence_reset(int newSeqNo, bool gapfill);
        Fix::Message logout(std::string text = {});



        private:
        void stamp_header_(Fix::Message& msg, std::string type);
        void stamp_trailer_(Fix::Message& msg);
        std::uint64_t compute_body_length_(Fix::Message& msg);
        inline std::uint32_t sum_bytes_(std::string_view v);
        inline std::uint32_t sum_tag_ascii_(int tag);
        std::uint8_t compute_check_sum_
        (Fix::Message& msg);
        constexpr int tag_count_(int tag) const noexcept;
        SessionParameters& params_;
        SeqProvider& seq_provider_;
        Clock& clock_;

    };

}
