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

TEST(MessageFactoryTests, AbsentFieldsMessageRegenTest) {
    Fix::SessionParameters params{};
    params.fix_version = "FIX.4.4";
    params.sender_comp_id = "SENDER";
    params.target_comp_id = "TARGET";

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

    std::string_view original_sv = 
        "8=FIX.4.4\x01"
        "9=65\x01"
        "35=2\x01"
        "34=5\x01"
        "49=SENDER\x01"
        "56=TARGET\x01"
        "52=20240605-11:59:59.000\x01"
        "7=10\x01"
        "16=20\x01"
        "10=200\x01";

    
    auto msg_index = Fix::MessageStore::create_message_index(original_sv, 0);

    std::string_view regen_msg = factory.regenerate_message(original_sv, msg_index);

    std::string_view expected_sv = 
        "8=FIX.4.4\x01"
        "9=97\x01"
        "35=2\x01"
        "34=5\x01"
        "49=SENDER\x01"
        "56=TARGET\x01"
        "52=20240606-12:00:00.000\x01"
        "7=10\x01"
        "16=20\x01"
        "43=Y\x01"
        "122=20240605-11:59:59.000\x01"
        "10=047\x01";

    

    EXPECT_EQ(regen_msg, expected_sv);
}

TEST(MessageFactoryTests, PresentFieldsMessageRegenTest) {
    Fix::SessionParameters params{};
    params.fix_version = "FIX.4.4";
    params.sender_comp_id = "SENDER";
    params.target_comp_id = "TARGET";

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

    std::string_view original_sv = 
        "8=FIX.4.4\x01"
        "9=81\x01"
        "35=2\x01"
        "34=5\x01"
        "49=SENDER\x01"
        "56=TARGET\x01"
        "52=20240605-11:59:59.000\x01"
        "43=N\x01"
        "122=20240605-11:59:58.000\x01"
        "7=10\x01"
        "16=20\x01"
        "10=150\x01";

    
    auto msg_index = Fix::MessageStore::create_message_index(original_sv, 0);

    std::string_view regen_msg = factory.regenerate_message(original_sv, msg_index);

    std::string_view expected_sv = 
        "8=FIX.4.4\x01"
        "9=97\x01"
        "35=2\x01"
        "34=5\x01"
        "49=SENDER\x01"
        "56=TARGET\x01"
        "52=20240606-12:00:00.000\x01"
        "43=Y\x01"
        "122=20240605-11:59:59.000\x01"
        "7=10\x01"
        "16=20\x01"
        "10=047\x01";

    

    EXPECT_EQ(regen_msg, expected_sv);
}