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



    
    

}
