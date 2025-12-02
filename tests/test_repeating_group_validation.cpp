#include <iostream>

#include <fix/core/Validator.hpp>
#include <fix/message/GenericMessage.hpp>
#include <fix/schema/Field.hpp>

Fix::Message::GenericMessage group1 = {
    {627, "2"},
    {628, "HOP1_COMP"},
    {629, "20251202-12:00:01.000"},
    {630, "1001"},
    {628, "HOP2_COMP"},
    {629, "20251202-12:00:02.000"},
    {630, "1002"}
};

Fix::Schema::FieldSchema raw_group_schema[] = { 
    {628, Fix::Schema::FieldType::STRING, Fix::Schema::FieldPresence::OPTIONAL},
    {629, Fix::Schema::FieldType::STRING, Fix::Schema::FieldPresence::OPTIONAL},
    {630, Fix::Schema::FieldType::INT,    Fix::Schema::FieldPresence::OPTIONAL}
};

Fix::Schema::GroupSchema group_schema = {
    raw_group_schema,
    3
};

Fix::Schema::FieldSchema group_field = {627, Fix::Schema::FieldType::GROUP, Fix::Schema::FieldPresence::OPTIONAL, &group_schema
};



int main() {
    Fix::Validator v{};

    int curr_index = 1; // Start after the 627 field
    auto results = v.validate_groups_(2, group1, &group_field, curr_index);

    if (results.empty()) {
        std::cout << "Group validation succeeded.\n";
        return 0; // Success
    } else {
        std::cout << "Group validation failed.\n";
        return 1; // Failure
    }
}