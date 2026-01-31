#include <gtest/gtest.h>
#include <fix/core/Validator.hpp>
#include <fix/core/Message.hpp>



TEST(AdminSchemasTests, LogonSchemaValidity) {
    Fix::Validator validator;
    Fix::ValidatorResult result;

    Fix::GenericMessage<Fix::GenericFieldView> valid_logon = {
        {"FIX.4.4", 8},
        {"100", 9},
        {"A", 35},
        {"SERVER1", 49},
        {"CLIENT1", 56}, 
        {"1", 34},
        {"20251224-09:30:00.000", 52},
        {"0", 98},
        {"30", 108},
        {"Y", 141},
        {"5", 789},
        {"2", 383},
        {"2", 384},
        {"A32", 372},
        {"N", 385},
        {"B116", 372},
        {"C", 385},
        {"235", 10}
    };

    Fix::ValidMessage valid_message = Fix::make_valid_message(valid_logon);

    Fix::SessionParameters params;
    params.sender_comp_id = "CLIENT1";
    params.target_comp_id = "SERVER1";

    result = validator.validate_message(valid_message, params);

    EXPECT_EQ(result.errors.size(), 0);
    EXPECT_EQ(result.severity, Fix::Error::Severity::Moderate);
}

TEST(AdminSchemasTests, HeartbeatSchemaValidity) {
    Fix::Validator validator;
    Fix::ValidatorResult result;

    Fix::GenericMessage<Fix::GenericFieldView> valid_heartbeat = {
        {"FIX.4.4", 8},
        {"80", 9},
        {"0", 35},
        {"SERVER1", 49},
        {"CLIENT1", 56},
        {"2", 34},
        {"20251224-09:30:05.000", 52},
        {"12345", 112},
        {"10", 10}
    };

    Fix::ValidMessage valid_message = Fix::make_valid_message(valid_heartbeat);

    Fix::SessionParameters params;
    params.sender_comp_id = "CLIENT1";
    params.target_comp_id = "SERVER1";

    result = validator.validate_message(valid_message, params);
    EXPECT_EQ(result.errors.size(), 0);
    EXPECT_EQ(result.severity, Fix::Error::Severity::Moderate);
}


TEST(AdminSchemasTests, TestRequestSchemaValidity) {
    Fix::Validator validator;
    Fix::ValidatorResult result;

    Fix::GenericMessage<Fix::GenericFieldView> valid_test_request = {
        {"FIX.4.4", 8},
        {"90", 9},
        {"1", 35},
        {"SERVER1", 49},
        {"CLIENT1", 56},
        {"3", 34},
        {"20251224-09:30:10.000", 52},
        {"REQ123", 112},
        {"10", 10}
    };

    Fix::ValidMessage valid_message = Fix::make_valid_message(valid_test_request);

    Fix::SessionParameters params;
    params.sender_comp_id = "CLIENT1";
    params.target_comp_id = "SERVER1";

    result = validator.validate_message(valid_message, params);
    EXPECT_EQ(result.errors.size(), 0);
    EXPECT_EQ(result.severity, Fix::Error::Severity::Moderate);
}

TEST(AdminSchemasTests, SequenceResetSchemaValidity) {
    Fix::Validator validator;
    Fix::ValidatorResult result;

    Fix::GenericMessage<Fix::GenericFieldView> valid_sequence_reset = {
        {"FIX.4.4", 8},
        {"110", 9},
        {"4", 35},
        {"SERVER1", 49},
        {"CLIENT1", 56},
        {"4", 34},
        {"20251224-09:30:15.000", 52},
        {"1500", 36},
        {"Y", 123},
        {"10", 10}
    };

    Fix::ValidMessage valid_message = Fix::make_valid_message(valid_sequence_reset);

    Fix::SessionParameters params;
    params.sender_comp_id = "CLIENT1";
    params.target_comp_id = "SERVER1";

    result = validator.validate_message(valid_message, params);
    EXPECT_EQ(result.errors.size(), 0);
    EXPECT_EQ(result.severity, Fix::Error::Severity::Moderate);
}

TEST(AdminSchemasTests, LogoutSchemaValidity) {
    Fix::Validator validator;
    Fix::ValidatorResult result;

    Fix::GenericMessage<Fix::GenericFieldView> valid_logout = {
        {"FIX.4.4", 8},
        {"90", 9},
        {"5", 35},
        {"SERVER1", 49},
        {"CLIENT1", 56},
        {"5", 34},
        {"20251224-09:30:20.000", 52},
        {"Normal logout", 58},
        {"10", 10}
    };

    Fix::ValidMessage valid_message = Fix::make_valid_message(valid_logout);

    Fix::SessionParameters params;
    params.sender_comp_id = "CLIENT1";
    params.target_comp_id = "SERVER1";

    result = validator.validate_message(valid_message, params);
    EXPECT_EQ(result.errors.size(), 0);
    EXPECT_EQ(result.severity, Fix::Error::Severity::Moderate);
}


TEST(AdminSchemasTests, ResendRequestSchemaValidity) {
    Fix::Validator validator;
    Fix::ValidatorResult result;

    Fix::GenericMessage<Fix::GenericFieldView> valid_resend_request = {
        {"FIX.4.4", 8},
        {"110", 9},
        {"2", 35},
        {"SERVER1", 49},
        {"CLIENT1", 56},
        {"6", 34},
        {"20251224-09:30:25.000", 52},
        {"100", 7},
        {"200", 16},
        {"10", 10}
    };

    Fix::ValidMessage valid_message = Fix::make_valid_message(valid_resend_request);

    Fix::SessionParameters params;
    params.sender_comp_id = "CLIENT1";
    params.target_comp_id = "SERVER1";

    result = validator.validate_message(valid_message, params);
    EXPECT_EQ(result.errors.size(), 0);
    EXPECT_EQ(result.severity, Fix::Error::Severity::Moderate);
}

TEST(AdminSchemasTests, RejectSchemaValidity) {
    Fix::Validator validator;
    Fix::ValidatorResult result;

    Fix::GenericMessage<Fix::GenericFieldView> valid_reject = {
        {"FIX.4.4", 8},
        {"110", 9},
        {"3", 35},
        {"SERVER1", 49},
        {"CLIENT1", 56},
        {"7", 34},
        {"20251224-09:30:30.000", 52},
        {"5", 45},
        {"2", 372},
        {"Invalid MsgType", 58},
        {"10", 10}
    };

    Fix::ValidMessage valid_message = Fix::make_valid_message(valid_reject);

    Fix::SessionParameters params;
    params.sender_comp_id = "CLIENT1";
    params.target_comp_id = "SERVER1";

    result = validator.validate_message(valid_message, params);
    EXPECT_EQ(result.errors.size(), 0);
    EXPECT_EQ(result.severity, Fix::Error::Severity::Moderate);
}