#pragma once
#include <string>
#include <array>
#include <vector>


namespace Fix::Schema {

    using Tag = std::size_t;

    struct GroupSchema; // Forward declaration

    enum class FieldType {
        INT,
        STRING,
        BOOLEAN,
        FLOAT,
        DOUBLE,
        CHAR,
        GROUP
    };

    enum class FieldPresence {
        REQUIRED,
        OPTIONAL,
        CONDITIONAL,
    };


    
    struct FieldSchema {
        Tag tag;
        FieldType type;
        FieldPresence presence;
        const GroupSchema* group_schema = nullptr;
    };

    struct GroupSchema {
        const FieldSchema* fields = nullptr;
        std::size_t field_count = 0;
    };


    inline bool is_double_type(std::string_view sv) noexcept {
        int first_num_idx = 0;
        if (sv.empty()) {
            return false;
        }
        if (sv[0] == '+' || sv[0] == '-') {
            first_num_idx++;
        }

        if (!static_cast<bool>(std::isdigit(sv[first_num_idx]))) {
            return false;
        }

        bool singledot = false;

        for (int i = first_num_idx+1; i < sv.size(); i++) {
            if (sv[i] == '.' && !singledot) {
                singledot = true;
            } else if (!static_cast<bool>(std::isdigit(sv[i]))) {
                return false;
            }
        }
        return true;
    }


    inline bool is_boolean_type(std::string_view sv) noexcept {
        return sv == "Y" || sv == "N";
    }

    inline bool is_char_type(std::string_view sv) noexcept {
        return sv.size() == 1;
    }

    inline bool is_int_type(std::string_view sv) noexcept {
        int first_num_idx = 0;
        if (sv.empty()) {
            return false;
        }
        if (sv[0] == '+' || sv[0] == '-') {
            first_num_idx++;
        }

        if (first_num_idx >= sv.size()) {
            return false;
        }

        for (int i = first_num_idx; i < sv.size(); i++) {
            if (!static_cast<bool>(std::isdigit(sv[i]))) {
                return false;
            }
        }
        return true;
    }



    
    

}
