#include <gtest/gtest.h>
#include <string_view>
#include <fix/core/MessageFactory.hpp>


TEST(MessageFactoryTests, LogonMessageTest) {
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

TEST(MessageFactoryTests, SequenceResetMessageTest) {
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
    std::string_view seq_reset_msg = factory.sequence_reset(10, true);

    const char expected[] = 
        "8=FIX.4.4\x01"
        "9=67\x01"
        "35=4\x01"
        "34=1\x01"
        "49=SENDER\x01"
        "56=TARGET\x01"
        "52=20240606-12:00:00.000\x01"
        "123=Y\x01"
        "36=10\x01"
        "10=129\x01"; 

    

    std::string_view expected_sv{expected, sizeof(expected) - 1};
    EXPECT_EQ(seq_reset_msg, expected_sv);
}

TEST(MessageFactoryTests, RejectMessageTest) {
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
    std::string_view reject_msg = factory.reject(5, 2, 45, "Invalid MsgSeqNum");

    const char expected[] = 
        "8=FIX.4.4\x01"
        "9=94\x01"
        "35=3\x01"
        "34=1\x01"
        "49=SENDER\x01"
        "56=TARGET\x01"
        "52=20240606-12:00:00.000\x01"
        "45=5\x01"
        "371=45\x01"
        "373=2\x01"
        "58=Invalid MsgSeqNum\x01"
        "10=136\x01"; 

    

    std::string_view expected_sv{expected, sizeof(expected) - 1};
    EXPECT_EQ(reject_msg, expected_sv);
}   

// Additional tests can be added for other message types as needed.