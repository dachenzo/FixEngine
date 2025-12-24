#pragma once
#include <charconv>
#include <vector>
#include <tuple>
#include <fix/core/utils.hpp>
#include <fix/error/ValidatorErrors.hpp>
#include <fix/schema/Field.hpp>
#include <fix/message/GenericMessage.hpp>
#include <fix/message/Header.hpp>
#include <fix/core/definitions.hpp>
#include <fix/schema/Registry.hpp>


namespace Fix {
    using ValidatorResult = std::vector<std::tuple<Error::Validator, std::size_t>>;
    


    struct Validator
    {

        // All functions will eventually return Validaro Result;

        void validate(const Message::GenericMessage& message);

        void validate_header_(const Message::GenericMessage& message, std::string& expected_message_type, ValidatorResult& validres);

        void validate_trailer_(const Message::GenericMessage& message, ValidatorResult& validres);

        ValidatorResult validate_message_body_(const Message::GenericMessage& message, std::string& expected_message_type);

        void validate_fields_(const Message::GenericMessage& message, const Schema::FieldSchema* schema, std::size_t schema_size, ValidatorResult& validres);

        void validate_groups_(const std::size_t groupcnt, const Message::GenericMessage& message, const Schema::FieldSchema* groupfield, int& curr_idx, ValidatorResult& validres);

        bool validate_type_(const std::string& value, Fix::Schema::FieldType type);


        private:
        Schema::Registry registry_ = {};

        






    };
}