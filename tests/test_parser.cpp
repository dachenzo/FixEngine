#include <gtest/gtest.h>
#include <fix/core/Parser.hpp>

using namespace Fix;

TEST(ParserTests, ParseValidLogonMessage) {
    Parser parser;
    const std::string kFixLogon =
        "8=FIX.4.4\x01"
        "9=77\x01"
        "35=A\x01"
        "34=1\x01"
        "49=CLIENT12\x01"
        "52=20251007-15:42:39.255\x01"
        "56=EXECUTOR\x01"
        "98=0\x01"
        "108=30\x01"
        "141=Y\x01"
        "10=199\x01";

    std::string_view sv(kFixLogon);
    
    ParseResult result = parser.parse(sv);
    EXPECT_TRUE(result.errs.empty());
    EXPECT_TRUE(result.message.has_value());

    Message::GenericMessage expected_message = {
        {8, "FIX.4.4"},
        {9, "77"},
        {35, "A"},
        {34, "1"},
        {49, "CLIENT12"},
        {52, "20251007-15:42:39.255"},
        {56, "EXECUTOR"},
        {98, "0"},
        {108, "30"},
        {141, "Y"},
        {10, "199"}
    };
    if (result.message.has_value()) {
        const auto& msg = result.message.value();
        EXPECT_EQ(msg.size(), expected_message.size());
        for (size_t i = 0; i < expected_message.size(); ++i) {
            EXPECT_EQ(msg[i].tag, expected_message[i].tag);
            EXPECT_EQ(msg[i].value, expected_message[i].value);
        }
    }
}


TEST(ParserTests, NoTag) {
    Parser parser;
    std::string raw_message = 
    "=FIX.4.4" "\x01" "9=69" "\x01";
    std::string_view sv(raw_message);
    
    ParseResult result = parser.parse(sv);
    ASSERT_FALSE(result.errs.empty());
    EXPECT_FALSE(result.message.has_value());

    EXPECT_EQ(result.errs[0], Error::Parse::NoTag);
}

TEST(ParserTests, MissingEqualSign) {
    Parser parser;
    std::string raw_message = "8FIX.4.4" "\x01" "9=69" "\x01";
    std::string_view sv(raw_message);
    
    ParseResult result = parser.parse(sv);
    ASSERT_FALSE(result.errs.empty());
    EXPECT_FALSE(result.message.has_value());

    EXPECT_EQ(result.errs[0], Error::Parse::MalformedTag);
}

TEST(ParserTests, MissingValue) {
    Parser parser;
    std::string raw_message = "8=" "\x01" "9=69" "\x01";
    std::string_view sv(raw_message);       
    ParseResult result = parser.parse(sv);
    ASSERT_FALSE(result.errs.empty());
    EXPECT_FALSE(result.message.has_value());

    EXPECT_EQ(result.errs[0], Error::Parse::MissingValue);
}

TEST(ParserTests, MalformedTag) {
    Parser parser;
    std::string raw_message = "8z=FIX.4.4" "\x01" "9=69" "\x01";
    std::string_view sv(raw_message);       
    ParseResult result = parser.parse(sv);
    ASSERT_FALSE(result.errs.empty());
    EXPECT_FALSE(result.message.has_value());

    EXPECT_EQ(result.errs[0], Error::Parse::MalformedTag);
}

TEST(ParserTests, WrongBodyLength) {
    Parser parser;
    const std::string kFixLogon =
        "8=FIX.4.4\x01"
        "9=76\x01" // Incorrect body length
        "35=A\x01"
        "34=1\x01"
        "49=CLIENT12\x01"
        "52=20251007-15:42:39.255\x01"
        "56=EXECUTOR\x01"
        "98=0\x01"
        "108=30\x01"
        "141=Y\x01"
        "10=198\x01";

    std::string_view sv(kFixLogon);
    
    ParseResult result = parser.parse(sv);
    ASSERT_FALSE(result.errs.empty());
    EXPECT_FALSE(result.message.has_value());

    EXPECT_EQ(result.errs[0], Error::Parse::Wrong_body_length);
}

TEST(ParserTests, FailedChecksum) {
    Parser parser;
    std::string raw_message = 
        "8=FIX.4.4\x01"
        "9=77\x01"
        "35=A\x01"
        "34=1\x01"
        "49=CLIENT12\x01"
        "52=20251007-15:42:39.255\x01"
        "56=EXECUTOR\x01"
        "98=0\x01"
        "108=30\x01"
        "141=Y\x01"
        "10=000\x01";  // Incorrect checksum

    std::string_view sv(raw_message);
    
    ParseResult result = parser.parse(sv);
    ASSERT_FALSE(result.errs.empty());
    EXPECT_FALSE(result.message.has_value());

    EXPECT_EQ(result.errs[0], Error::Parse::Failed_checksum);
}   