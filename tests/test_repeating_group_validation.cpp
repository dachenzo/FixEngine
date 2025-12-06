#include <iostream>
#include <string>

#include <fix/core/Validator.hpp>
#include <fix/message/GenericMessage.hpp>
#include <fix/schema/Field.hpp>

int goodgroup1cnt = 2;
Fix::Message::GenericMessage goodgroup1 = {
    {627, std::to_string(goodgroup1cnt)},
    {628, "HOP1_COMP"},
    {629, "20251202-12:00:01.000"},
    {630, "1001"},
    {628, "HOP2_COMP"},
    {629, "20251202-12:00:02.000"},
    {630, "1002"}
};

int badgroup1cnt = 2;
Fix::Message::GenericMessage badgroup1 = { // Missing a reuqired field (628)
    {627, std::to_string(badgroup1cnt)},
    {629, "HOP1_COMP"},
    {630, "1001"}, 
    {628, "HOP2_COMP"},
    {629, "20251202-12:00:02.000"},
    {630, "1002"}
};


int badgroup2cnt = 2;
Fix::Message::GenericMessage badgroup2 = { // Wrong type for 630
    {627, std::to_string(badgroup2cnt)},
    {628, "HOP1_COMP"},
    {629, "20251202-12:00:01.000"},
    {630, "NOT_AN_INT"}, 
    {628, "HOP2_COMP"},
    {629, "20251202-12:00:02.000"},
    {630, "1002"}
};

int badgroup3cnt = 3;
Fix::Message::GenericMessage badgroup3 = { // undersized group count
    {627, std::to_string(badgroup3cnt)},
    {628, "HOP1_COMP"},
    {629, "20251202-12:00:01.000"},
    {630, "1001"},
    {628, "HOP2_COMP"},
    {629, "20251202-12:00:02.000"},
    {630, "1002"},
};


//oversized group count
int badgroup4cnt = 5;
Fix::Message::GenericMessage badgroup4 = {
    {627, std::to_string(badgroup4cnt)},
    {628, "HOP1_COMP"},
    {629, "20251202-12:00:01.000"},
    {630, "1001"},
    {628, "HOP2_COMP"},
    {629, "20251202-12:00:02.000"},
    {630, "1002"},
};


// repeated field in group
int badgroup5cnt = 3;
Fix::Message::GenericMessage badgroup5 = {
    {627, std::to_string(badgroup5cnt)},
    {628, "HOP1_COMP"},
    {629, "20251202-12:00:01.000"},
    {630, "1001"},
    {630, "1001"},
    {628, "HOP1_COMP"}, 
    {629, "20251202-12:00:02.000"},
    {630, "1002"},
};


Fix::Schema::FieldSchema raw_group_schema[] = { 
    {628, Fix::Schema::FieldType::STRING, Fix::Schema::FieldPresence::REQUIRED},
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

    int curr_index = 1; // start after the group count field    
    auto res = v.validate_groups_(goodgroup1cnt, goodgroup1, &group_field, curr_index);
    if (res.empty()) {
        std::cout << "goodgroup1 passed validation\n";
    } else {
        std::cout << "goodgroup1 failed validation\n";  
    }

    curr_index = 1;
    res = v.validate_groups_(badgroup1cnt, badgroup1, &group_field, curr_index);
    if (res.empty()) {
        std::cout << "badgroup1 passed validation\n";
    } else {
        std::cout << "badgroup1 failed validation\n"; 
    }

    curr_index = 1;
    res = v.validate_groups_(badgroup2cnt, badgroup2, &group_field, curr_index);
    if (res.empty()) {
        std::cout << "badgroup2 passed validation\n";
    } else {
        std::cout << "badgroup2 failed validation\n"; 
    }

    curr_index = 1;
    res = v.validate_groups_(badgroup3cnt, badgroup3, &group_field, curr_index);
    if (res.empty()) {
        std::cout << "badgroup3 passed validation\n";
    } else {
        std::cout << "badgroup3 failed validation\n"; 
    }

    curr_index = 1;
    res = v.validate_groups_(badgroup4cnt, badgroup4, &group_field, curr_index);
    if (res.empty()) {
        std::cout << "badgroup4 passed validation\n";
    } else {
        std::cout << "badgroup4 failed validation\n"; 
    }
    curr_index = 1;
    res = v.validate_groups_(badgroup5cnt, badgroup5, &group_field, curr_index);
    if (res.empty()) {
        std::cout << "badgroup5 passed validation\n";
    } else {
        std::cout << "badgroup5 failed validation\n"; 
    }

    return 0;
}