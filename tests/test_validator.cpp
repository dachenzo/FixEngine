#include <gtest/gtest.h>
#include <fix/core/Validator.hpp>


using namespace Fix;

TEST(ValidatorTests, UnknownMessageType) {
    Validator validator;
    Fix::SessionParameters params {
        .sender_comp_id = "me",
        .target_comp_id = "them"
    };;
    GenericMessage<GenericFieldView> logon_min = {
        {"FIX.4.4", 8},
        {"69", 9},
        {"l", 35},                       // Logon
        {"them", 49},
        {"me", 56},
        {"1", 34},
        {"20251224-09:30:00.000", 52},
        {"0", 98},                       // EncryptMethod (0 = None)
        {"30", 108},                      // HeartBtInt
        {"235", 10}
    };

    auto valid_logon = Fix::make_valid_message(logon_min);
    
    ValidatorResult result = validator.validate_message(valid_logon, params);

    EXPECT_EQ(result.errors.size(), 1UL);
    EXPECT_EQ(std::get<0>(result.errors[0]), Error::Validator::UnknownMessageType);
}


TEST(ValidatorTests, WrongFixVersion) {
    Validator validator;
    Fix::SessionParameters params {
        .sender_comp_id = "me",
        .target_comp_id = "them"
    };;
    GenericMessage<GenericFieldView> logon_bad_fix = {
        {"FIX.4.2", 8},
        {"69", 9},
        {"A", 35},                       // Logon
        {"them", 49},
        {"me", 56},
        {"1", 34},
        {"20251224-09:30:00.000", 52},
        {"0", 98},                       // EncryptMethod (0 = None)
        {"30", 108},                      // HeartBtInt
        {"235", 10}
    };
    auto valid_logon_bad_fix = Fix::make_valid_message(logon_bad_fix);
    ValidatorResult result = validator.validate_message(valid_logon_bad_fix, params);

    EXPECT_EQ(result.errors.size(), 1UL);
    EXPECT_EQ(std::get<0>(result.errors[0]), Error::Validator::WrongFixVersion);
}

TEST(ValidatorTests, WrongFieldType) {
    Validator validator;
    Fix::SessionParameters params {
        .sender_comp_id = "me",
        .target_comp_id = "them"
    };
    GenericMessage<GenericFieldView> logon_bad_field = {
        {"FIX.4.4", 8},
        {"69", 9},
        {"A", 35},                       // Logon
        {"them", 49},
        {"me", 56},
        {"1", 34},
        {"20251224-09:30:00.000", 52},
        {"INVALID_INT", 98},             // EncryptMethod (0 = None)
        {"30", 108},                      // HeartBtInt
        {"235", 10}
    };
    auto valid_logon_bad_field = Fix::make_valid_message(logon_bad_field);
    ValidatorResult result = validator.validate_message(valid_logon_bad_field, params);

    EXPECT_EQ(result.errors.size(), 1UL);
    EXPECT_EQ(std::get<0>(result.errors[0]), Error::Validator::WrongFieldType);
}

TEST(ValidatorTests, MissingField) {
    Validator validator;
    Fix::SessionParameters params {
        .sender_comp_id = "me",
        .target_comp_id = "them"
    };
    GenericMessage<GenericFieldView> logon_missing_field = {
        {"FIX.4.4", 8},
        {"69", 9},
        {"A", 35},                       // Logon
        {"them", 49},
        {"me", 56},
        {"1", 34},
        {"20251224-09:30:00.000", 52},
        // Missing EncryptMethod (98)
        {"30", 108},                      // HeartBtInt
        {"235", 10}
    };

    auto valid_logon_missing_field = Fix::make_valid_message(logon_missing_field);
    ValidatorResult result = validator.validate_message(valid_logon_missing_field, params);

    EXPECT_EQ(result.errors.size(), 1UL);
    EXPECT_EQ(std::get<0>(result.errors[0]), Error::Validator::MissingField);
}


TEST(ValidatorTests, MissingGroupEntry) {
    Validator validator;
  
    GenericMessage<GenericFieldView> message_missing_group_entry = {
        {"FIX.4.4", 8},
        {"100", 9},
        {"A", 35},                       // Logon
        {"them", 49},
        {"me", 56},
        {"1", 34},
        {"20251224-09:30:00.000", 52},
        {"0", 98},                       // EncryptMethod (0 = None)
        {"30", 108},                      // HeartBtInt
        {"2", 627},                      // NoHops
        // Missing group entries here
        {"235", 10}
    };
    
    Fix::SessionParameters params {
        .sender_comp_id = "me",
        .target_comp_id = "them"
    };

    auto valid_message_missing_group_entry = Fix::make_valid_message(message_missing_group_entry);
    ValidatorResult result = validator.validate_message(valid_message_missing_group_entry, params);

    ASSERT_FALSE(result.errors.empty());
    EXPECT_EQ(std::get<0>(result.errors[0]), Error::Validator::MissingGroupEntry);
}


TEST(ValidatorTests, UnrecognizedField) {
    Validator validator;
    Fix::SessionParameters params {
        .sender_comp_id = "me",
        .target_comp_id = "them"
    };
    GenericMessage<GenericFieldView> message_missing_group_schema = {
        {"FIX.4.4", 8},
        {"100", 9},
        {"A", 35},                       // Logon
        {"them", 49},
        {"me", 56},
        {"1", 34},
        {"20251224-09:30:00.000", 52},
        {"0", 98},                       // EncryptMethod (0 = None)
        {"30", 108},                      // HeartBtInt
        {"1", 627},                      // NoHops
        {"20251202-12:00:01.000", 629},  // HopSendingTime (out of order)
        {"HOP1_COMP", 628},              // HopCompID, unrecognized field because of wrong order
        {"1001", 630},                   // HopRefID , unrecognized field  because of wrong order
        {"235", 10}
    };
    
    auto valid_message_missing_group_schema = Fix::make_valid_message(message_missing_group_schema);
    ValidatorResult result = validator.validate_message(valid_message_missing_group_schema, params);

    EXPECT_EQ(result.errors.size(), 1UL);
    EXPECT_EQ(std::get<0>(result.errors[0]), Error::Validator::UnrecognizedField);
}


TEST(ValidatorTests, MalformedTag34) {
    Validator validator;
    Fix::SessionParameters params {
        .sender_comp_id = "me",
        .target_comp_id = "them"
    };
    GenericMessage<GenericFieldView> logon_bad_seqnum = {
        {"FIX.4.4", 8},
        {"79", 9},
        {"A", 35},                       // Logon
        {"them", 49},
        {"me", 56},
        {"INVALID_INT", 34},             // MsgSeqNum
        {"20251224-09:30:00.000", 52},
        {"0", 98},                       // EncryptMethod (0 = None)
        {"30", 108},                      // HeartBtInt
        {"012", 10}
    };
    auto valid_logon_bad_seqnum = Fix::make_valid_message(logon_bad_seqnum);
    ValidatorResult result = validator.validate_message(valid_logon_bad_seqnum, params);

    EXPECT_EQ(result.errors.size(), 1UL);
    EXPECT_EQ(result.severity, Error::Severity::Fatal);
    EXPECT_EQ(std::get<0>(result.errors[0]), Error::Validator::WrongFieldType);
}

TEST(ValidatorTests, MalformedTag8) {
    Validator validator;
    Fix::SessionParameters params {
        .sender_comp_id = "me",
        .target_comp_id = "them"
    };
    GenericMessage<GenericFieldView> logon_bad_seqnum = {
        {"88", 8},
        {"70", 9},
        {"A", 35},                       // Logon
        {"them", 49},
        {"me", 56},
        {"88", 34},             // MsgSeqNum
        {"20251224-09:30:00.000", 52},
        {"0", 98},                       // EncryptMethod (0 = None)
        {"30", 108},                      // HeartBtInt
        {"231", 10}
    };

    auto valid_logon_bad_seqnum = Fix::make_valid_message(logon_bad_seqnum);
    ValidatorResult result = validator.validate_message(valid_logon_bad_seqnum, params);

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
    GenericMessage<GenericFieldView> logon_bad_sender = {
        {"FIX.4.4", 8},
        {"78", 9},
        {"A", 35},                       // Logon
        {"not_target", 49},            // Wrong SenderCompID
        {"me", 56},
        {"1", 34},
        {"20251224-09:30:00.000", 52},
        {"0", 98},                       // EncryptMethod (0 = None)
        {"30", 108},                      // HeartBtInt
        {"166", 10}
    };
    auto valid_logon_bad_sender = Fix::make_valid_message(logon_bad_sender);
    ValidatorResult result = validator.validate_message(valid_logon_bad_sender, params);

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
    GenericMessage<GenericFieldView> logon_bad_target = {
        {"FIX.4.4", 8},
        {"78", 9},
        {"A", 35},                       // Logon
        {"them", 49},
        {"not_me", 56},            // Wrong TargetCompID
        {"1", 34},
        {"20251224-09:30:00.000", 52},
        {"0", 98},                       // EncryptMethod (0 = None)
        {"30", 108},                      // HeartBtInt
        {"166", 10}
    };

    auto valid_logon_bad_target = Fix::make_valid_message(logon_bad_target);
    ValidatorResult result = validator.validate_message(valid_logon_bad_target, params);

    EXPECT_EQ(result.errors.size(), 1UL);
    EXPECT_EQ(result.severity, Error::Severity::Fatal);
    EXPECT_EQ(std::get<0>(result.errors[0]), Error::Validator::WrongTargetCompID);
}
