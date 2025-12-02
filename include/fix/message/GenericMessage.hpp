#pragma once
#include <vector>
#include <string>
#include <tuple>


namespace Fix::Message {

    struct GenericField {
        std::size_t tag;
        std::string value;
    };


    struct GenericMessage {
        std::vector<GenericField> fields;

        inline std::vector<GenericField>::const_iterator find(std::size_t tag) const {
            for (auto it = fields.begin(); it < fields.end(); it++) if (it->tag == tag) return it;
            return fields.end();
        }

        inline std::vector<GenericField>::const_iterator find(std::size_t tag, std::size_t from) const {
            for (auto it = fields.begin()+from; it < fields.end(); it++) if (it->tag == tag) return it;
            return fields.end();
        }

        inline const std::vector<GenericField>& get_fields() const noexcept {return fields;}

        



        inline std::vector<GenericField>::const_iterator end() const  noexcept {return fields.end();}
    };
}