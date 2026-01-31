#pragma once
#include <array>
#include <string_view>
#include <fix/schema/Field.hpp>

namespace Fix::Message {

    using namespace Fix::Schema;

    inline constexpr std::array<FieldSchema, 2> SequenceResetSchema {{
            {36, FieldType::INT,    FieldPresence::REQUIRED}, // NewSeqNo
            {123, FieldType::BOOLEAN, FieldPresence::OPTIONAL}  // GapFillFlag
    }};

    
    struct SequenceReset {
        inline static constexpr std::string_view MsgType = "4";
        inline static constexpr  const FieldSchema* Schema = SequenceResetSchema.data();
        inline static constexpr  const std::size_t SchemaSize = SequenceResetSchema.size();        
    };
}