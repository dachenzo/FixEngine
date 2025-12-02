#include <algorithm>
#include <fix/core/Validator.hpp>



namespace Fix {
    
    ValidatorResult Validator::validate_header_(const Message::GenericMessage& message, std::string& expected_message_type) {
        auto& schema = Message::StandardHeaderSchema;

        if (message.size() < 3) {
            //automatically invalid 
        }


        if (!validate_type_(message[0].value, schema[0].type) || message[0].value != Fix::DEFAULT_FIX_VERSION) {
            // wrong fix version
        }

        if (!validate_type_(message[1].value, schema[1].type)) {
            // invalid body length
        }   

        if (!validate_type_(message[2].value, schema[2].type) || message[2].value != expected_message_type) {
            // invalid msg type
        }


        auto results = validate_fields_(message, schema.data(), schema.size());
        return results;

    }


    ValidatorResult Validator::validate_fields_(const Message::GenericMessage& message, const Schema::FieldSchema* schema, std::size_t schema_size) {
        ValidatorResult results{};
        for (std::size_t i = 0; i < schema_size; i++) {
            const auto& field_schema = schema[i];

            auto it = std::find_if(message.begin(), message.end(), [&](const Message::GenericField& field) {
                return field.tag == field_schema.tag;
            });
            if (it != message.end()) {
                if (field_schema.type == Schema::FieldType::GROUP) {
                    int start_idx = static_cast<int>(it - message.begin());
                    std::size_t group_count = 0;
                    if (!Fix::Utils::parse_int(it->value, group_count)) {
                        results.emplace_back(Error::Validator::WrongFieldType, field_schema.tag);
                        return results;
                    }
                    auto res = validate_groups_(group_count, message, &field_schema, start_idx);
                    results.insert(results.end(), res.begin(), res.end());

                } else if (!validate_type_(it->value, field_schema.type)) {
                    results.emplace_back(Error::Validator::WrongFieldType, field_schema.tag);
                    return results;
                }

            } else if ( field_schema.presence == Schema::FieldPresence::REQUIRED) {
                results.emplace_back(Error::Validator::MissingField, field_schema.tag);
                return results;
            }
            
            
        }
        return results;

    };

    ValidatorResult Validator::validate_groups_(const std::size_t groupcnt, const Message::GenericMessage& message, const 
    Schema::FieldSchema* groupfield, int& curr_index) {
        ValidatorResult results{};
    

        if (groupcnt == 0) return results;
        

        if (!groupfield->group_schema) {
            results.emplace_back(Error::Validator::MissingGroupSchemaEntry, groupfield->tag);
            return results;
        };



        auto& gs = groupfield->group_schema;
        

        for (std::size_t i = 0; i < groupcnt; i++) {
            for (std::size_t j = 0; j < gs->field_count; j++) {
                const auto& field_schema = gs->fields[j];

                if (message.size() <= curr_index) {
                    results.emplace_back(Error::Validator::MissingGroupEntry, field_schema.tag);
                    return results;
                }

                if (message[curr_index].tag != field_schema.tag) {
                    if (field_schema.presence == Schema::FieldPresence::REQUIRED) {
                        results.emplace_back(Error::Validator::MissingGroupEntryOrWrongOrder, field_schema.tag);
                        return results;
                    }
                } else {
                    if (field_schema.type == Schema::FieldType::GROUP) {
                        curr_index++;
                        std::size_t group_count = 0;
                        if (!Fix::Utils::parse_int(message[curr_index-1].value, group_count)) {
                            results.emplace_back(Error::Validator::WrongFieldType, field_schema.tag);
                            return results;
                        }
                        auto res = validate_groups_(group_count, message, &field_schema, curr_index);
                        results.insert(results.end(), res.begin(), res.end());

                    } else if (!validate_type_(message[curr_index].value, field_schema.type)) {
                        results.emplace_back(Error::Validator::WrongFieldType, field_schema.tag);
                        return results;
                    }
                    curr_index++;
                } 
            }
        } 
        
        return results;
        
    };



    bool Validator::validate_type_(const std::string& value, Fix::Schema::FieldType type) {
        
        switch (type)
        {
        case Fix::Schema::FieldType::GROUP:    
        case Fix::Schema::FieldType::INT: 
            return Schema::is_int_type(value);
        case Fix::Schema::FieldType::STRING:
            return true;
        case Fix::Schema::FieldType::BOOLEAN:
            return Schema::is_boolean_type(value);
        case Fix::Schema::FieldType::FLOAT:
            return Schema::is_double_type(value);
        case Fix::Schema::FieldType::DOUBLE:
            return Schema::is_double_type(value);
        case Fix::Schema::FieldType::CHAR:
            return Schema::is_char_type(value);  
        default:
            break;

        
        };
        return false;

    }
}