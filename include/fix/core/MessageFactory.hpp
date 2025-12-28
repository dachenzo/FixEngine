#pragma once
#include <optional>
#include <string>
#include <cstdint>
#include <string_view>
#include <fix/core/definitions.hpp>
#include <fix/core/Clock.hpp>
#include <fix/core/SeqProvider.hpp>
#include <fix/core/Message.hpp>

namespace Fix {

    struct MessageFactory {
        MessageFactory(SessionParameters& params, SeqProvider& seq_provider, Clock& clock);


        Fix::ValidMessage logon(int heartbeat_override, bool echo_reset);
        Fix::ValidMessage heart_beat(std::optional<std::string> test_req_id = std::nullopt);
        Fix::ValidMessage test_request(std::string id);
        Fix::ValidMessage resend_request(int begin_seq_no, int end_seq_no);
        Fix::ValidMessage sequence_reset(int newSeqNo, bool gapfill);
        Fix::ValidMessage logout(std::string text = {});



        private:
        void stamp_header_(Fix::ValidMessage& msg, std::string type);
        void stamp_trailer_(Fix::ValidMessage& msg);
        std::uint64_t compute_body_length_(Fix::ValidMessage& msg);
        inline std::uint32_t sum_bytes_(std::string_view v);
        inline std::uint32_t sum_tag_ascii_(int tag);
        std::uint8_t compute_check_sum_
        (Fix::ValidMessage& msg);
        constexpr int tag_count_(int tag) const noexcept;
        SessionParameters& params_;
        SeqProvider& seq_provider_;
        Clock& clock_;

    };

}
