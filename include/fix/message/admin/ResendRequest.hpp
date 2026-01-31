#pragma once
#include <array>
#include <string_view>
#include <fix/schema/Field.hpp>


namespace  Fix::Message {

    using namespace Fix::Schema;

    inline constexpr std::array<FieldSchema, 2> ResendRequestSchema {{
            {7,  FieldType::INT,    FieldPresence::REQUIRED}, // BeginSeqNo
            {16, FieldType::INT,    FieldPresence::REQUIRED}  // EndSeqNo
    }};

    
    struct ResendRequest {
        inline static constexpr std::string_view MsgType = "2";
        inline static constexpr  const FieldSchema* Schema = ResendRequestSchema.data();
        inline static constexpr  const std::size_t SchemaSize = ResendRequestSchema.size();        
    };

}