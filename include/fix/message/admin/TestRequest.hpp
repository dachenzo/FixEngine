#pragma once
#include <array>
#include <string_view>
#include <fix/schema/Field.hpp>


namespace Fix::Message {

    using namespace Fix::Schema;

    inline constexpr std::array<FieldSchema, 1> TestRequestSchema {{
            {112, FieldType::STRING, FieldPresence::OPTIONAL} // TestReqID
    }};

    
    struct TestRequest {
        inline static constexpr std::string_view MsgType = "1";
        inline static constexpr  const FieldSchema* Schema = TestRequestSchema.data();
        inline static constexpr  const std::size_t SchemaSize = TestRequestSchema.size();
    }; 

}