#pragma once
#include <array>
#include <string_view>
#include <fix/schema/Field.hpp>


namespace Fix::Message {

    using namespace Fix::Schema;

    inline constexpr std::array<FieldSchema, 1> HeartbeatSchema {{
            {112, FieldType::STRING, FieldPresence::OPTIONAL} // TestReqID
    }};

    
    struct Heartbeat {
        inline static constexpr std::string_view MsgType = "0";
        inline static constexpr  const FieldSchema* Schema = HeartbeatSchema.data();
        inline static constexpr  const std::size_t SchemaSize = HeartbeatSchema.size();
    }; 

}