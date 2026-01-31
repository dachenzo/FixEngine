#pragma once 
#include <array>
#include <string_view>
#include <fix/schema/Field.hpp>

namespace Fix::Message {

    using namespace Fix::Schema;

    inline constexpr std::array<FieldSchema, 3> LogoutSchema {{
            {58, FieldType::STRING, FieldPresence::OPTIONAL}, // Text
            {354, FieldType::INT, FieldPresence::OPTIONAL},    // SessionRejectReason Length
            {355, FieldType::STRING, FieldPresence::OPTIONAL}
    }};





    struct Logout {
        inline static constexpr std::string_view MsgType = "5";
        inline static constexpr  const FieldSchema* Schema = LogoutSchema.data();
        inline static constexpr  const std::size_t SchemaSize = LogoutSchema.size();
    }; 

}