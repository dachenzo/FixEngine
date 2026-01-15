#include <gtest/gtest.h>
#include <fix/core/Framer.hpp>


TEST(FramerTests, BasicMessageFraming) {
    Fix::Framer framer;


    std::string_view msg1 = "8=FIX.4.4\x01""9=25\x01""35=A\x01""49=SENDER\x01""56=TARGET\x01""10=000\x01";
    std::string_view msg2 = "8=FIX.4.4\x01""9=27\x01""35=B\x01""49=SENDER2\x01""56=TARGET2\x01""10=000\x01";

    framer.append(msg1);
    EXPECT_TRUE(framer.has_message());
    auto framed_msg1 = framer.get_message();
    EXPECT_EQ(framed_msg1, msg1);
    framer.consume_message();
    EXPECT_FALSE(framer.has_message());

    framer.append(msg2);
    EXPECT_TRUE(framer.has_message());
    auto framed_msg2 = framer.get_message();
    EXPECT_EQ(framed_msg2, msg2);
    framer.consume_message();
    EXPECT_FALSE(framer.has_message());

}


TEST(FramerTests, GarbageBeforeAndAfterMessage) {
    Fix::Framer framer;

    std::string_view msg = "GARBAGE_DATA_BEFORE""8=FIX.4.4\x01""9=25\x01""35=A\x01""49=SENDER\x01""56=TARGET\x01""10=000\x01""GARBAGE_DATA_AFTER";
    framer.append(msg);
    EXPECT_TRUE(framer.has_message());
    auto framed_msg = framer.get_message();
    std::string_view expected_msg = "8=FIX.4.4\x01""9=25\x01""35=A\x01""49=SENDER\x01""56=TARGET\x01""10=000\x01";
    EXPECT_EQ(framed_msg, expected_msg);
    framer.consume_message();
    EXPECT_FALSE(framer.has_message());
}

TEST(FramerTests, PartialMessageSplitOnTag8) {
    Fix::Framer framer;

    std::string_view msg = "GARBAGE_DATA_BEFORE""8=FI";

    std::string_view msg_part2 = 
    "X.4.4\x01""9=25\x01""35=A\x01""49=SENDER\x01""56=TARGET\x01""10=000\x01""GARBAGE_DATA_AFTER";
    framer.append(msg);
    EXPECT_FALSE(framer.has_message());
    framer.append(msg_part2);
    EXPECT_TRUE(framer.has_message());
    auto framed_msg = framer.get_message();
    std::string_view expected_msg = "8=FIX.4.4\x01""9=25\x01""35=A\x01""49=SENDER\x01""56=TARGET\x01""10=000\x01";
    EXPECT_EQ(framed_msg, expected_msg);
    framer.consume_message();
    EXPECT_FALSE(framer.has_message());
}

TEST(FramerTests, PartialMessageSplitOnTag9) {
    Fix::Framer framer;

    std::string_view msg = "GARBAGE_DATA_BEFORE""8=FIX.4.4\x01""9=";

    std::string_view msg_part2 = 
    "25\x01""35=A\x01""49=SENDER\x01""56=TARGET\x01""10=000\x01""GARBAGE_DATA_AFTER";
    framer.append(msg);
    EXPECT_FALSE(framer.has_message());
    framer.append(msg_part2);
    EXPECT_TRUE(framer.has_message());
    auto framed_msg = framer.get_message();
    std::string_view expected_msg = "8=FIX.4.4\x01""9=25\x01""35=A\x01""49=SENDER\x01""56=TARGET\x01""10=000\x01";
    EXPECT_EQ(framed_msg, expected_msg);
    framer.consume_message();
    EXPECT_FALSE(framer.has_message());
}


TEST(FramerTests, MultipleMessagesInSingleAppend) {
    Fix::Framer framer;

    std::string_view combined_msgs = 
    "8=FIX.4.4\x01""9=27\x01""35=A\x01""49=SENDER1\x01""56=TARGET1\x01""10=000\x01"
    "GARBAGE_BETWEEN_MESSAGES"
    "8=FIX.4.4\x01""9=27\x01""35=B\x01""49=SENDER2\x01""56=TARGET2\x01""10=000\x01";

    framer.append(combined_msgs);
    EXPECT_TRUE(framer.has_message());
    auto framed_msg1 = framer.get_message();
    std::string_view expected_msg1 = "8=FIX.4.4\x01""9=27\x01""35=A\x01""49=SENDER1\x01""56=TARGET1\x01""10=000\x01";
    EXPECT_EQ(framed_msg1, expected_msg1);
    framer.consume_message();
    EXPECT_TRUE(framer.has_message());
    auto framed_msg2 = framer.get_message();
    std::string_view expected_msg2 = "8=FIX.4.4\x01""9=27\x01""35=B\x01""49=SENDER2\x01""56=TARGET2\x01""10=000\x01";
    EXPECT_EQ(framed_msg2, expected_msg2);
    framer.consume_message();
    EXPECT_FALSE(framer.has_message());
}


TEST(FramerTests, IncompleteMessageNotFramed) {
    Fix::Framer framer;
    std::string_view incomplete_msg = "8=FIX.4.4\x01""9=25\x01""35=A\x01""49=SENDER\x01"; // Missing 56 and 10 tags
    framer.append(incomplete_msg);
    EXPECT_FALSE(framer.has_message());
}


TEST(FramerTests, MalformedTagNineWithCorrectMessageAfter) {
    Fix::Framer framer;
    std::string malformed_msg_before_correct_msg = "8=FIX.4.4\x01""9=" + std::string(Fix::Framer::MAX_BEGIN_TO_BODYLEN_FIELD_BYTES, 'X') + "8=FIX.4.4\x01""9=27\x01""35=B\x01""49=SENDER2\x01""56=TARGET2\x01""10=000\x01";
    framer.append(malformed_msg_before_correct_msg);
    EXPECT_TRUE(framer.has_message());
    auto framed_msg = framer.get_message();
    std::string_view expected_msg = "8=FIX.4.4\x01""9=27\x01""35=B\x01""49=SENDER2\x01""56=TARGET2\x01""10=000\x01";
    EXPECT_EQ(framed_msg, expected_msg);
    framer.consume_message();
    EXPECT_FALSE(framer.has_message());


    //case2:
    Fix::Framer framer2;
    std::string malformed_msg_followed_by_malformed_msg = "8=FIX.4.4\x01""9=" + std::string(Fix::Framer::MAX_BEGIN_TO_BODYLEN_FIELD_BYTES, 'X') + "8=FIX.4.4\x01""9=27\x01""35=B\x01""49=SENDER2\x01";
    framer2.append(malformed_msg_followed_by_malformed_msg);
    EXPECT_FALSE(framer2.has_message());

}


TEST(FramerTests, MalformedBodyLengthWithCorrectMessageAfter) {
    Fix::Framer framer;
    std::string malformed_body_length_msg_before_correct_msg = "8=FIX.4.4\x01""9=XYZ\x01""35=A\x01""49=SENDER\x01""56=TARGET\x01""10=000\x01""8=FIX.4.4\x01""9=27\x01""35=B\x01""49=SENDER2\x01""56=TARGET2\x01""10=000\x01";
    framer.append(malformed_body_length_msg_before_correct_msg);
    EXPECT_TRUE(framer.has_message());
    auto framed_msg = framer.get_message();
    std::string_view expected_msg = "8=FIX.4.4\x01""9=27\x01""35=B\x01""49=SENDER2\x01""56=TARGET2\x01""10=000\x01";
    EXPECT_EQ(framed_msg, expected_msg);
    framer.consume_message();
    EXPECT_FALSE(framer.has_message());
}


TEST(FramerTests, ExceedsMaxMessageSize) {
    Fix::Framer framer;

    // Create a large message close to MAX_MESSAGE_SIZE
    std::string large_body(Fix::Framer::MAX_MESSAGE_SIZE+1, 'A'); 
    std::string large_msg = "8=FIX.4.4\x01""9=" + std::to_string(large_body.size()) + "\x01" + large_body + "10=000\x01";

    framer.append(large_msg);
    EXPECT_FALSE(framer.has_message());
    // Now append a valid message
    std::string_view valid_msg = "8=FIX.4.4\x01""9=27\x01""35=B\x01""49=SENDER2\x01""56=TARGET2\x01""10=000\x01";
    framer.append(valid_msg);
    EXPECT_TRUE(framer.has_message());
    auto framed_msg = framer.get_message();
    EXPECT_EQ(framed_msg, valid_msg);
    framer.consume_message();
    EXPECT_FALSE(framer.has_message());
}

TEST(FramerTests, MalformedMessageByTagNinePosition) {
    Fix::Framer framer;

    std::string_view malformed_msg = "8=FIX.4.4\x01""GARBAGE_DATA_IN_MIDDLE""9=25\x01""35=A\x01""49=SENDER\x01""56=TARGET\x01""10=000\x01";
    framer.append(malformed_msg);
    EXPECT_FALSE(framer.has_message());

    std::string_view valid_msg = "8=FIX.4.4\x01""9=27\x01""35=B\x01""49=SENDER2\x01""56=TARGET2\x01""10=000\x01";
    framer.append(valid_msg);
    EXPECT_TRUE(framer.has_message());
    auto framed_msg = framer.get_message();
    EXPECT_EQ(framed_msg, valid_msg);
    framer.consume_message();
    EXPECT_FALSE(framer.has_message());
}