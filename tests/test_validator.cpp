#include <gtest/gtest.h>
#include <fix/core/Validator.hpp>


using namespace Fix;

TEST(ValidatorTests, UnknownMessageType) {
    Validator validator;
    Message::GenericMessage logon_min = {
        {8,  "FIX.4.4"},
        {9,  "69"},
        {35, "A"},                       // Logon
        {49, "CLIENT1"},
        {56, "SERVER1"},
        {34, "1"},
        {52, "20251224-09:30:00.000"},
        {98, "0"},                       // EncryptMethod (0 = None)
        {108,"30"},                      // HeartBtInt
        {10, "235"}
    };
    std::string expected_message_type = "Z";
    ValidatorResult result = validator.validate_message(logon_min, expected_message_type);

    EXPECT_EQ(result.size(), 1UL);
    EXPECT_EQ(std::get<0>(result[0]), Error::Validator::UnknownMessageType);
}


TEST(ValidatorTest, WrongFixVersion) {
    Validator validator;
    Message::GenericMessage logon_bad_fix = {
        {8,  "FIX.4.2"},
        {9,  "69"},
        {35, "A"},                       // Logon
        {49, "CLIENT1"},
        {56, "SERVER1"},
        {34, "1"},
        {52, "20251224-09:30:00.000"},
        {98, "0"},                       // EncryptMethod (0 = None)
        {108,"30"},                      // HeartBtInt
        {10, "235"}
    };
    std::string expected_message_type = "A";
    ValidatorResult result = validator.validate_message(logon_bad_fix, expected_message_type);

    EXPECT_EQ(result.size(), 1UL);
    EXPECT_EQ(std::get<0>(result[0]), Error::Validator::WrongFixVersion);
}

TEST(ValidatorTest, WrongFieldType) {
    Validator validator;
    Message::GenericMessage logon_bad_field = {
        {8,  "FIX.4.4"},
        {9,  "69"},
        {35, "A"},                       // Logon
        {49, "CLIENT1"},
        {56, "SERVER1"},
        {34, "1"},
        {52, "20251224-09:30:00.000"},
        {98, "INVALID_INT"},             // EncryptMethod (0 = None)
        {108,"30"},                      // HeartBtInt
        {10, "235"}
    };
    std::string expected_message_type = "A";
    ValidatorResult result = validator.validate_message(logon_bad_field, expected_message_type);

    EXPECT_EQ(result.size(), 1UL);
    EXPECT_EQ(std::get<0>(result[0]), Error::Validator::WrongFieldType);
}
TEST(ValidatorTest, MissingField) {
    Validator validator;
    Message::GenericMessage logon_missing_field = {
        {8,  "FIX.4.4"},
        {9,  "69"},
        {35, "A"},                       // Logon
        {49, "CLIENT1"},
        {56, "SERVER1"},
        {34, "1"},
        {52, "20251224-09:30:00.000"},
        // Missing EncryptMethod (98)
        {108,"30"},                      // HeartBtInt
        {10, "235"}
    };
    std::string expected_message_type = "A";
    ValidatorResult result = validator.validate_message(logon_missing_field, expected_message_type);

    EXPECT_EQ(result.size(), 1UL);
    EXPECT_EQ(std::get<0>(result[0]), Error::Validator::MissingField);
}


TEST(ValidatorTest, MissingGroupEntry) {
    Validator validator;
    Message::GenericMessage message_missing_group_entry = {
        {8,  "FIX.4.4"},
        {9,  "100"},
        {35, "A"},                       // Logon
        {49, "CLIENT1"},
        {56, "SERVER1"},
        {34, "1"},
        {52, "20251224-09:30:00.000"},
        {98, "0"},                       // EncryptMethod (0 = None)
        {108,"30"},                      // HeartBtInt
        {627, "2"},                      // NoHops
        // Missing group entries here
        {10, "235"}
    };
    std::string expected_message_type = "A";
    ValidatorResult result = validator.validate_message(message_missing_group_entry, expected_message_type);

    ASSERT_FALSE(result.empty());
    EXPECT_EQ(std::get<0>(result[0]), Error::Validator::MissingGroupEntry);
}


TEST(ValidatorTest, UnrecognizedField) {
    Validator validator;
    Message::GenericMessage message_missing_group_schema = {
        {8,  "FIX.4.4"},
        {9,  "100"},
        {35, "A"},                       // Logon
        {49, "CLIENT1"},
        {56, "SERVER1"},
        {34, "1"},
        {52, "20251224-09:30:00.000"},
        {98, "0"},                       // EncryptMethod (0 = None)
        {108,"30"},                      // HeartBtInt
        {627, "1"},                      // NoHops
        {629, "20251202-12:00:01.000"},  // HopSendingTime (out of order)
        {628, "HOP1_COMP"},              // HopCompID, unrecognized field because of wrong order
        {630, "1001"},                   // HopRefID , unrecognized field  because of wrong order
        {10, "235"}
    };
    std::string expected_message_type = "A";
    ValidatorResult result = validator.validate_message(message_missing_group_schema, expected_message_type);

    EXPECT_EQ(result.size(), 1UL);
    EXPECT_EQ(std::get<0>(result[0]), Error::Validator::UnrecognizedField);
}
