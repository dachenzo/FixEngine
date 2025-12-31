#include <gtest/gtest.h>
#include <fix/core/Validator.hpp>


using namespace Fix;

TEST(ValidatorTests, UnknownMessageType) {
    Validator validator;
    Fix::SessionParameters params {
        .sender_comp_id = "me",
        .target_comp_id = "them"
    };;
    Message::GenericMessage logon_min = {
        {8,  "FIX.4.4"},
        {9,  "69"},
        {35, "l"},                       // Logon
        {49, "them"},
        {56, "me"},
        {34, "1"},
        {52, "20251224-09:30:00.000"},
        {98, "0"},                       // EncryptMethod (0 = None)
        {108,"30"},                      // HeartBtInt
        {10, "235"}
    };
    
    ValidatorResult result = validator.validate_message(logon_min, params);

    EXPECT_EQ(result.errors.size(), 1UL);
    EXPECT_EQ(std::get<0>(result.errors[0]), Error::Validator::UnknownMessageType);
}


TEST(ValidatorTests, WrongFixVersion) {
    Validator validator;
    Fix::SessionParameters params {
        .sender_comp_id = "me",
        .target_comp_id = "them"
    };;
    Message::GenericMessage logon_bad_fix = {
        {8,  "FIX.4.2"},
        {9,  "69"},
        {35, "A"},                       // Logon
        {49, "them"},
        {56, "me"},
        {34, "1"},
        {52, "20251224-09:30:00.000"},
        {98, "0"},                       // EncryptMethod (0 = None)
        {108,"30"},                      // HeartBtInt
        {10, "235"}
    };
    std::string expected_message_type = "A";
    ValidatorResult result = validator.validate_message(logon_bad_fix, params);

    EXPECT_EQ(result.errors.size(), 1UL);
    EXPECT_EQ(std::get<0>(result.errors[0]), Error::Validator::WrongFixVersion);
}

TEST(ValidatorTests, WrongFieldType) {
    Validator validator;
    Fix::SessionParameters params {
        .sender_comp_id = "me",
        .target_comp_id = "them"
    };;
    Message::GenericMessage logon_bad_field = {
        {8,  "FIX.4.4"},
        {9,  "69"},
        {35, "A"},                       // Logon
        {49, "them"},
        {56, "me"},
        {34, "1"},
        {52, "20251224-09:30:00.000"},
        {98, "INVALID_INT"},             // EncryptMethod (0 = None)
        {108,"30"},                      // HeartBtInt
        {10, "235"}
    };
    ValidatorResult result = validator.validate_message(logon_bad_field, params);

    EXPECT_EQ(result.errors.size(), 1UL);
    EXPECT_EQ(std::get<0>(result.errors[0]), Error::Validator::WrongFieldType);
}

TEST(ValidatorTests, MissingField) {
    Validator validator;
    Fix::SessionParameters params {
        .sender_comp_id = "me",
        .target_comp_id = "them"
    };;
    Message::GenericMessage logon_missing_field = {
        {8,  "FIX.4.4"},
        {9,  "69"},
        {35, "A"},                       // Logon
        {49, "them"},
        {56, "me"},
        {34, "1"},
        {52, "20251224-09:30:00.000"},
        // Missing EncryptMethod (98)
        {108,"30"},                      // HeartBtInt
        {10, "235"}
    };
    ValidatorResult result = validator.validate_message(logon_missing_field, params);

    EXPECT_EQ(result.errors.size(), 1UL);
    EXPECT_EQ(std::get<0>(result.errors[0]), Error::Validator::MissingField);
}


TEST(ValidatorTests, MissingGroupEntry) {
    Validator validator;
  
    Message::GenericMessage message_missing_group_entry = {
        {8,  "FIX.4.4"},
        {9,  "100"},
        {35, "A"},                       // Logon
        {49, "them"},
        {56, "me"},
        {34, "1"},
        {52, "20251224-09:30:00.000"},
        {98, "0"},                       // EncryptMethod (0 = None)
        {108,"30"},                      // HeartBtInt
        {627, "2"},                      // NoHops
        // Missing group entries here
        {10, "235"}
    };
    
    Fix::SessionParameters params {
        .sender_comp_id = "me",
        .target_comp_id = "them"
    };;
    ValidatorResult result = validator.validate_message(message_missing_group_entry, params);

    ASSERT_FALSE(result.errors.empty());
    EXPECT_EQ(std::get<0>(result.errors[0]), Error::Validator::MissingGroupEntry);
}


TEST(ValidatorTests, UnrecognizedField) {
    Validator validator;
    Fix::SessionParameters params {
        .sender_comp_id = "me",
        .target_comp_id = "them"
    };;
    Message::GenericMessage message_missing_group_schema = {
        {8,  "FIX.4.4"},
        {9,  "100"},
        {35, "A"},                       // Logon
        {49, "them"},
        {56, "me"},
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
    
    ValidatorResult result = validator.validate_message(message_missing_group_schema, params);

    EXPECT_EQ(result.errors.size(), 1UL);
    EXPECT_EQ(std::get<0>(result.errors[0]), Error::Validator::UnrecognizedField);
}


TEST(ValidatorTests, MalformedTag34) {
    Validator validator;
    Fix::SessionParameters params {
        .sender_comp_id = "me",
        .target_comp_id = "them"
    };;
    Message::GenericMessage logon_bad_seqnum = {
        {8,  "FIX.4.4"},
        {9,  "79"},
        {35, "A"},                       // Logon
        {49, "them"},
        {56, "me"},
        {34, "INVALID_INT"},             // MsgSeqNum
        {52, "20251224-09:30:00.000"},
        {98, "0"},                       // EncryptMethod (0 = None)
        {108,"30"},                      // HeartBtInt
        {10, "012"}
    };
    ValidatorResult result = validator.validate_message(logon_bad_seqnum, params);

    EXPECT_EQ(result.errors.size(), 1UL);
    EXPECT_EQ(result.severity, Error::Severity::Fatal);
    EXPECT_EQ(std::get<0>(result.errors[0]), Error::Validator::WrongFieldType);
}

TEST(ValidatorTests, MalformedTag8) {
    Validator validator;
    Fix::SessionParameters params {
        .sender_comp_id = "me",
        .target_comp_id = "them"
    };;
    Message::GenericMessage logon_bad_seqnum = {
        {8,  "88"},
        {9,  "70"},
        {35, "A"},                       // Logon
        {49, "them"},
        {56, "me"},
        {34, "88"},             // MsgSeqNum
        {52, "20251224-09:30:00.000"},
        {98, "0"},                       // EncryptMethod (0 = None)
        {108,"30"},                      // HeartBtInt
        {10, "231"}
    };
    ValidatorResult result = validator.validate_message(logon_bad_seqnum, params);

    EXPECT_EQ(result.errors.size(), 1UL);
    EXPECT_EQ(result.severity, Error::Severity::Fatal);
    EXPECT_EQ(std::get<0>(result.errors[0]), Error::Validator::WrongFixVersion);
}

TEST(ValidatorTests, WrongSenderCompID) {
    Validator validator;
    Fix::SessionParameters params {
        .sender_comp_id = "me",
        .target_comp_id = "them"
    };
    Message::GenericMessage logon_bad_sender = {
        {8,  "FIX.4.4"},
        {9,  "78"},
        {35, "A"},                       // Logon
        {49, "not_target"},            // Wrong SenderCompID
        {56, "me"},
        {34, "1"},
        {52, "20251224-09:30:00.000"},
        {98, "0"},                       // EncryptMethod (0 = None)
        {108,"30"},                      // HeartBtInt
        {10, "166"}
    };
    ValidatorResult result = validator.validate_message(logon_bad_sender, params);

    EXPECT_EQ(result.errors.size(), 1UL);
    EXPECT_EQ(result.severity, Error::Severity::Fatal);
    EXPECT_EQ(std::get<0>(result.errors[0]), Error::Validator::WrongSenderCompID);
}

TEST(ValidatorTests, WrongTargetCompID) {
    Validator validator;
    Fix::SessionParameters params {
        .sender_comp_id = "me",
        .target_comp_id = "them"
    };
    Message::GenericMessage logon_bad_target = {
        {8,  "FIX.4.4"},
        {9,  "78"},
        {35, "A"},                       // Logon
        {49, "them"},
        {56, "not_me"},            // Wrong TargetCompID
        {34, "1"},
        {52, "20251224-09:30:00.000"},
        {98, "0"},                       // EncryptMethod (0 = None)
        {108,"30"},                      // HeartBtInt
        {10, "166"}
    };
    ValidatorResult result = validator.validate_message(logon_bad_target, params);

    EXPECT_EQ(result.errors.size(), 1UL);
    EXPECT_EQ(result.severity, Error::Severity::Fatal);
    EXPECT_EQ(std::get<0>(result.errors[0]), Error::Validator::WrongTargetCompID);
}
