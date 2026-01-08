#include <gtest/gtest.h>
#include <fix/core/MessageFactory.hpp>


TEST(MessageFactoryTests, LogonMessageTest) {
    Fix::SessionParameters params{};
    params.fix_version = "FIX.4.4";
    params.sender_comp_id = "SENDER";
    params.target_comp_id = "TARGET";
    params.encrypt_method_str = "0";
    params.heart_beat_str = "30";

    Fix::SeqProvider seq_provider{};
    struct ClockMock {
        std::string now_fix() {
            return "20240606-12:00:00.000";
        }
        std::uint64_t now() {
            return 0;
        }
    };
    ClockMock clock{};
    Fix::MessageFactory<ClockMock> factory{params, seq_provider, clock};
    std::string_view logon_msg = factory.logon(30, true);

    const char expected[] = 
        "8=FIX.4.4\x01"
        "9=73\x01"
        "35=A\x01"
        "34=1\x01"
        "49=SENDER\x01"
        "56=TARGET\x01"
        "52=20240606-12:00:00.000\x01"
        "98=0\x01"
        "141=Y\x01"
        "108=30\x01"
        "10=156\x01"; 

    std::string_view expected_sv{expected, sizeof(expected) - 1};


    EXPECT_EQ(logon_msg, expected_sv);
}