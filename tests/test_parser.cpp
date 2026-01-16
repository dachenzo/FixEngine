#include <gtest/gtest.h>
#include <fix/core/Parser.hpp>
#include <fix/message/GenericMessage.hpp>


TEST(ParserTests, BasicValidMessage) {
    Fix::Parser parser;
    Fix::GenericMessage<Fix::GenericFieldView> msg;
    std::vector<Fix::ParseErrorInfo> errors;

    std::string_view valid_msg = "8=FIX.4.4\x01""9=25\x01""35=A\x01""49=SENDER\x01""56=TARGET\x01""10=194\x01";
    parser.parse(valid_msg, msg, errors);

    EXPECT_TRUE(errors.empty());
    EXPECT_EQ(msg.size(), 6);
    EXPECT_EQ(msg[0].tag, 8);
    EXPECT_EQ(msg[0].value, "FIX.4.4");
    EXPECT_EQ(msg[1].tag, 9);
    EXPECT_EQ(msg[1].value, "25");
    EXPECT_EQ(msg[2].tag, 35);
    EXPECT_EQ(msg[2].value, "A");
    EXPECT_EQ(msg[3].tag, 49);
    EXPECT_EQ(msg[3].value, "SENDER");
    EXPECT_EQ(msg[4].tag, 56);
    EXPECT_EQ(msg[4].value, "TARGET");
    EXPECT_EQ(msg[5].tag, 10);
    EXPECT_EQ(msg[5].value, "194");

}