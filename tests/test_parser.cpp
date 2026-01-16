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


TEST(ParserTests, InvalidChecksumWrongDigit) {
    Fix::Parser parser;
    Fix::GenericMessage<Fix::GenericFieldView> msg;
    std::vector<Fix::ParseErrorInfo> errors;

    std::string_view invalid_checksum_msg = "8=FIX.4.4\x01""9=25\x01""35=A\x01""49=SENDER\x01""56=TARGET\x01""10=000\x01";
    parser.parse(invalid_checksum_msg, msg, errors);

    EXPECT_FALSE(errors.empty());
    EXPECT_EQ(errors.size(), 1);
    EXPECT_EQ(errors[0].tag, 10);
    EXPECT_EQ(errors[0].code, Fix::ParseError::Failed_checksum);
}

TEST(ParserTests, MissingChecksum) {
    Fix::Parser parser;
    Fix::GenericMessage<Fix::GenericFieldView> msg;
    std::vector<Fix::ParseErrorInfo> errors;

    std::string_view missing_checksum_msg = "8=FIX.4.4\x01""9=25\x01""35=A\x01""49=SENDER\x01""56=TARGET\x01";
    parser.parse(missing_checksum_msg, msg, errors);

    EXPECT_FALSE(errors.empty());
    EXPECT_EQ(errors.size(), 1);
    EXPECT_EQ(errors[0].tag, 10);
    EXPECT_EQ(errors[0].code, Fix::ParseError::Failed_checksum);
}

TEST(ParserTests, MalformedTag) {
    Fix::Parser parser;
    Fix::GenericMessage<Fix::GenericFieldView> msg;
    std::vector<Fix::ParseErrorInfo> errors;

    std::string_view malformed_tag_msg = "8=FIX.4.4\x01"
                                        "9=30\x01"
                                        "35=A\x01"
                                        "49=SENDER\x01"
                                        "56=TARGET\x01"
                                        "AB=1\x01"      // malformed tag (non-numeric)
                                        "10=176\x01";

    parser.parse(malformed_tag_msg, msg, errors);

    EXPECT_FALSE(errors.empty());
    EXPECT_EQ(errors.size(), 1);
    EXPECT_EQ(errors[0].tag, Fix::Parser::undefined_tag);
    EXPECT_EQ(errors[0].code, Fix::ParseError::MalformedTag);
}

