#include <algorithm>
#include <fix/core/Validator.hpp>



namespace Fix {
    
    void Validator::validate_header_(const Message::GenericMessage& message, const std::string& expected_message_type,  ValidatorResult& results) {
        auto& schema = Message::StandardHeaderSchema;
      

        if (message.size() < 3) {
            //automatically invalid 
            results.emplace_back(Error::Validator::MissingField, 0);
            return;
        }


        if (!validate_type_(message[0].value, schema[0].type) || message[0].value != Fix::DEFAULT_FIX_VERSION) {
            // wrong fix versio
            results.emplace_back(Error::Validator::WrongFixVersion, 0);
            return;
        }

        if (!validate_type_(message[1].value, schema[1].type)) {
            // invalid body length
            results.emplace_back(Error::Validator::WrongFieldType, schema[1].tag);
            return;
        }   

        if (!validate_type_(message[2].value, schema[2].type) || message[2].value != expected_message_type) {
            // invalid msg type
            results.emplace_back(Error::Validator::WrongFieldType, schema[2].tag);
            return;
        }


        validate_fields_(message, schema.data(), schema.size(), results);
        return;

    }

    ValidatorResult Validator::validate_message(const Message::GenericMessage& message, const std::string& expected_message_type) {
        // Placeholder implementation
        ValidatorResult results{};
        tagscratch_.ensure_bits(message.size());
        tagscratch_.clear();

        auto schema = registry_.get(expected_message_type);
        if (!schema) {
            results.push_back({Error::Validator::UnknownMessageType, 0});
            return results;
        }
        validate_header_(message, expected_message_type, results);
        validate_fields_(message, schema->body, schema->body_field_count, results);
        validate_trailer_(message, results);

        // Only flag extra fields when no prior errors were found
        if (results.empty() && !tagscratch_.full(message.size())) {
            results.push_back({Error::Validator::UnrecognizedField, 0});
        }

        return results;
    }   

    void Validator::validate_fields_(const Message::GenericMessage& message, const Schema::FieldSchema* schema, std::size_t schema_size, ValidatorResult& results) {
        
        for (std::size_t i = 0; i < schema_size; i++) {
            const auto& field_schema = schema[i];

            auto it = std::find_if(message.begin(), message.end(), [&](const Message::GenericField& field) {
                return field.tag == field_schema.tag;
            });
            if (it != message.end()) {
                tagscratch_.set(it, message.begin());
                if (field_schema.type == Schema::FieldType::GROUP) {
                    int start_idx = static_cast<int>(it - message.begin())+1; //one to advance past the group count field
                    std::size_t group_count = 0;
                    if (!Fix::Utils::parse_int(it->value, group_count)) {
                        results.emplace_back(Error::Validator::WrongFieldType, field_schema.tag);
                        return;
                    }
                    validate_groups_(group_count, message, &field_schema, start_idx, results);

                } else if (!validate_type_(it->value, field_schema.type)) {
                    results.emplace_back(Error::Validator::WrongFieldType, field_schema.tag);
                    return;
                }

            } else if ( field_schema.presence == Schema::FieldPresence::REQUIRED) {
                results.emplace_back(Error::Validator::MissingField, field_schema.tag);
                return;
            }
            
            
        }
        return;

    };

    void Validator::validate_groups_(const std::size_t groupcnt, const Message::GenericMessage& message, const 
    Schema::FieldSchema* groupfield, int& curr_index, ValidatorResult& results) {
        
        constexpr std::size_t kMaxGroupCount = 32;

        if (groupcnt == 0) return;
        
        if(!groupfield) {
            results.emplace_back(Error::Validator::MissingGroupEntry, 0);
            return;
        }

        if (!groupfield->group_schema) {
            results.emplace_back(Error::Validator::MissingGroupSchemaEntry, groupfield->tag);
            return;
        };

        auto& gs = groupfield->group_schema;
        if (gs->field_count == 0 || gs->fields == nullptr) {
            results.emplace_back(Error::Validator::MissingGroupSchemaEntry, groupfield->tag);
            return;
        };


        if (gs->field_count > kMaxGroupCount) {
            results.emplace_back(Error::Validator::UnsupportedGroupSize, groupfield->tag);
            return;
        }

        struct FieldOcc {
            unsigned long tag;
            int cnt;
        };

        std::array<FieldOcc, kMaxGroupCount> count{};
        for (int i = 0; i < gs->field_count; i++) {
            count[i] = {gs->fields[i].tag, 0};
        }


        auto has_occured = [&gs, &count](unsigned int tag) {
            for (int i = 0; i < gs->field_count; i++) {
                if (tag == count[i].tag && count[i].cnt > 0) return true;
            }
            return false;
        };

        auto increment_tag_count = [&gs, &count](unsigned int tag) {
            for (int i = 0; i < gs->field_count; i++) {
                if (tag == count[i].tag)  count[i].cnt++;
            }
        };
 

        for (std::size_t i = 0; i < groupcnt; i++) {
            for (std::size_t i = 0; i < gs->field_count; ++i) count[i].cnt = 0;
            for (std::size_t j = 0; j < gs->field_count; j++) {
                const auto& field_schema = gs->fields[j];

                if (curr_index >= message.size()) {
                    results.emplace_back(Error::Validator::MissingGroupEntry, field_schema.tag);
                    return;
                }

                auto current_tag = message[curr_index].tag;


                if (current_tag != field_schema.tag) {
                    if (field_schema.presence == Schema::FieldPresence::REQUIRED) {
                        results.emplace_back(Error::Validator::MissingGroupEntryOrWrongOrder, field_schema.tag); 
                        return;
                    }
                } else if (has_occured(current_tag)) {
                    //assume this is the next repeating group starting or your done with repeating groups
                    break;
                } else {
                    tagscratch_.set(curr_index);
                    if (field_schema.type == Schema::FieldType::GROUP) {
                        curr_index++;
                        std::size_t group_count = 0;
                        if (!Fix::Utils::parse_int(message[curr_index-1].value, group_count)) {
                            results.emplace_back(Error::Validator::WrongFieldType, field_schema.tag);
                            return;
                        }
                        validate_groups_(group_count, message, &field_schema, curr_index, results);
                        

                    } else if (!validate_type_(message[curr_index].value, field_schema.type)) {
                        results.emplace_back(Error::Validator::WrongFieldType, field_schema.tag);
                        return;
                    }
                    increment_tag_count(current_tag);
                    curr_index++;
                } 
            }
            bool atleast_one_occurence = false;
            for (int i = 0; i < gs->field_count; i++) {
                if (count[i].cnt > 0)  atleast_one_occurence = true;
            } 

            if (!atleast_one_occurence) {
                results.emplace_back(Error::Validator::MissingGroupEntry, groupfield->tag);
                return;
            }
        } 
        
        return;
        
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

    void Validator::validate_trailer_(const Message::GenericMessage& message, ValidatorResult& results) {
        
        //This is done by the initial parser usually, so we skip it here
        return;
    }
}
