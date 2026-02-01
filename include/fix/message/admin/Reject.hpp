#pragma once
#include <array>
#include <string_view>
#include <fix/schema/Field.hpp>


namespace Fix::Message {

    using namespace Fix::Schema;

    inline constexpr std::array<FieldSchema, 7> RejectSchema {{
            {45, FieldType::INT,     FieldPresence::REQUIRED}, // RefSeqNum
            {371, FieldType::INT,    FieldPresence::OPTIONAL}, // RefTagID
            {372, FieldType::STRING,  FieldPresence::OPTIONAL}, // RefMsgType
            {373, FieldType::INT,     FieldPresence::OPTIONAL}, // SessionRejectReason
            {58, FieldType::STRING,   FieldPresence::OPTIONAL},  // Text
            {354, FieldType::INT,    FieldPresence::OPTIONAL}, // SessionRejectReason Length
            {355, FieldType::STRING,  FieldPresence::OPTIONAL}
    }};

    
    struct Reject {
        inline static constexpr std::string_view MsgType = "3";
        inline static constexpr  const FieldSchema* Schema = RejectSchema.data();
        inline static constexpr  const std::size_t SchemaSize = RejectSchema.size();        
    };

}