#pragma once
#include <array>
#include <string_view>
#include <fix/schema/Field.hpp>


namespace Fix::Message {

    using namespace Fix::Schema;

    inline constexpr std::array<FieldSchema, 1> CustomSchema {{
        {9250, FieldType::STRING,    FieldPresence::REQUIRED} 
    }};

    struct Custom {
        inline static constexpr std::string_view MsgType = "custom_admin";
        inline static constexpr  const FieldSchema* Schema = CustomSchema.data();
        inline static constexpr  const std::size_t SchemaSize = CustomSchema.size();        
    };

}