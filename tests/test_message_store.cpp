#include <gtest/gtest.h>
#include <fix/core/MessageStore.hpp>


TEST(MessageStoreTests, CreateMessageIndexTest) {
    Fix::MessageStore store{};
    std::string_view fix_msg = 
        "8=FIX.4.4\x01"
        "9=65\x01"
        "35=D\x01"
        "34=12\x01"
        "49=SENDER\x01"
        "56=TARGET\x01"
        "52=20240606-12:00:00.000\x01"
        "11=12345\x01"
        "21=1\x01"
        "43=Y\x01"
        "122=67890\x01"
        "55=EUR/USD\x01"
        "54=1\x01"
        "60=20240606-12:00:00.000\x01"
        "10=220\x01";

    auto index = store.create_message_index(fix_msg, 0);
    EXPECT_EQ(index.off, 0);
    EXPECT_EQ(index.len, fix_msg.size());
    EXPECT_EQ(index.seq, 12);
    EXPECT_EQ(index.get_msg_type(), "D");
    EXPECT_EQ(index.off_34, fix_msg.find("34=") + 3);
    EXPECT_EQ(index.off_52, fix_msg.find("52=") + 3);
    EXPECT_EQ(index.off_43, fix_msg.find("43=") + 3);
    EXPECT_EQ(index.off_122, fix_msg.find("122=") + 4);
    EXPECT_EQ(index.len_34, 2); // "12"
    EXPECT_EQ(index.len_52, 21); // "20240606-12:00:00.000"
    EXPECT_EQ(index.len_43, 1); // "Y"
    EXPECT_EQ(index.len_122, 5); // "67890"
}


TEST(MessageStoreTests, StoreOutboundMessageTest) {
    Fix::MessageStore store{};
    std::string_view fix_msg1 = 
        "8=FIX.4.4\x01"
        "9=65\x01"
        "35=D\x01"
        "34=1\x01"
        "49=SENDER\x01"
        "56=TARGET\x01"
        "52=20240606-12:00:00.000\x01"
        "11=12345\x01"
        "21=1\x01"
        "55=EUR/USD\x01"
        "54=1\x01"
        "60=20240606-12:00:00.000\x01"
        "10=220\x01";

    std::string_view fix_msg2 = 
        "8=FIX.4.4\x01"
        "9=66\x01"
        "35=D\x01"
        "34=2\x01"
        "49=SENDER\x01"
        "56=TARGET\x01"
        "52=20240606-12:05:00.000\x01"
        "11=12346\x01"
        "21=1\x01"
        "55=GBP/USD\x01"
        "54=2\x01"
        "60=20240606-12:05:00.000\x01"
        "10=221\x01";

    std::string_view fix_msg3 = 
        "8=FIX.4.4\x01"
        "9=64\x01"
        "35=0\x01"
        "34=3\x01"
        "49=SENDER\x01"
        "56=TARGET\x01"
        "52=20240606-12:10:00.000\x01"
        "10=222\x01";

    std::string_view fix_msg4 = 
        "8=FIX.4.4\x01"
        "9=64\x01"
        "35=0\x01"
        "34=4\x01"
        "49=SENDER\x01"
        "56=TARGET\x01"
        "52=20240606-12:10:00.000\x01"
        "10=222\x01";

    store.store_outbound_message(fix_msg1);
    store.store_outbound_message(fix_msg2);
    store.store_outbound_message(fix_msg3);
    store.store_outbound_message(fix_msg4);

    auto resend_stream = store.get_resend_stream(1, 4);
    EXPECT_TRUE(resend_stream.has_next());

    auto action1 = resend_stream.next();
    EXPECT_EQ(action1.begin_seq_no, 1);
    EXPECT_EQ(action1.end_seq_no, 1);
    EXPECT_FALSE(action1.gap_fill);

    EXPECT_TRUE(resend_stream.has_next());
    auto action2 =resend_stream.next();
    EXPECT_EQ(action2.begin_seq_no, 2);
    EXPECT_EQ(action2.end_seq_no, 2);
    EXPECT_FALSE(action2.gap_fill);

    EXPECT_TRUE(resend_stream.has_next());
    auto action3 = resend_stream.next();
    EXPECT_EQ(action3.begin_seq_no, 3);
    EXPECT_EQ(action3.end_seq_no, 4);
    EXPECT_TRUE(action3.gap_fill);

    EXPECT_FALSE(resend_stream.has_next());
}


TEST(MessageStoreTests, ResendStreamNoResendTest) {
    Fix::MessageStore store{};
    // Store messages with types that should not be resent
    for (uint32_t i = 1; i <= 5; ++i) {
        std::string fix_msg = 
            "8=FIX.4.4\x01"
            "9=50\x01"
            "35=0\x01" // Heartbeat
            "34=" + std::to_string(i) + "\x01"
            "49=SENDER\x01"
            "56=TARGET\x01"
            "52=20240606-12:00:00.000\x01"
            "10=220\x01";
        store.store_outbound_message(fix_msg);
    }

    auto resend_stream = store.get_resend_stream(1, 5);
    EXPECT_TRUE(resend_stream.has_next());

    auto action = resend_stream.next();
    EXPECT_EQ(action.begin_seq_no, 1);
    EXPECT_EQ(action.end_seq_no, 5);
    EXPECT_TRUE(action.gap_fill);

    EXPECT_FALSE(resend_stream.has_next());
}

TEST(MessageStoreTests, BlobLayoutTest) {
    Fix::MessageStore store{};
    std::string_view fix_msg1 = 
        "8=FIX.4.4\x01"
        "9=65\x01"
        "35=D\x01"
        "34=1\x01"
        "49=SENDER\x01"
        "56=TARGET\x01"
        "52=20240606-12:00:00.000\x01"
        "11=12345\x01"
        "21=1\x01"
        "55=EUR/USD\x01"
        "54=1\x01"
        "60=20240606-12:00:00.000\x01"
        "10=220\x01";

    std::string_view fix_msg2 = 
        "8=FIX.4.4\x01"
        "9=66\x01"
        "35=D\x01"
        "34=2\x01"
        "49=SENDER\x01"
        "56=TARGET\x01"
        "52=20240606-12:05:00.000\x01"
        "11=12346\x01"
        "21=1\x01"
        "55=GBP/USD\x01"
        "54=2\x01"
        "60=20240606-12:05:00.000\x01"
        "10=221\x01";

    store.store_outbound_message(fix_msg1);
    store.store_outbound_message(fix_msg2);
    
    auto all_msgs = store.all_messages();
    std::string expected_all_msgs = std::string(fix_msg1) + std::string(fix_msg2);
    EXPECT_EQ(all_msgs, expected_all_msgs);
}