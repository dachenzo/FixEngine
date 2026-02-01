#pragma once
#include <array>
#include <string_view>
#include <fix/schema/Field.hpp>


namespace Fix::Message {

    using namespace Fix::Schema;

    inline constexpr std::array<FieldSchema, 2> RawGroup384Schema {{
        {372, FieldType::STRING,    FieldPresence::REQUIRED}, 
        {385, FieldType::CHAR,  FieldPresence::REQUIRED}  
    }};

    inline constexpr GroupSchema Group384Schema {
        RawGroup384Schema.data(),
        RawGroup384Schema.size()
    };

    inline constexpr std::array<FieldSchema, 11> LogonSchema {{
            {98,  FieldType::INT,     FieldPresence::REQUIRED}, // EncryptMethod
            {108, FieldType::INT,     FieldPresence::REQUIRED}, // HeartBtInt
            {141, FieldType::BOOLEAN, FieldPresence::OPTIONAL}, // ResetSeqNumFlag
            {95,  FieldType::INT,     FieldPresence::OPTIONAL}, // RawDataLength
            {96,  FieldType::STRING,  FieldPresence::OPTIONAL}, // RawData
            {553, FieldType::STRING,  FieldPresence::OPTIONAL}, // Username
            {554, FieldType::STRING,  FieldPresence::OPTIONAL},  // Password
            {789, FieldType::INT, FieldPresence::OPTIONAL},
            {383, FieldType::INT, FieldPresence::OPTIONAL},
            {384, FieldType::GROUP, FieldPresence::OPTIONAL, &Group384Schema},
            {464, FieldType::BOOLEAN, FieldPresence::OPTIONAL}
    }};

    
    struct Logon {
        inline static constexpr std::string_view MsgType = "A";
        inline static constexpr  const FieldSchema* Schema = LogonSchema.data();
        inline static constexpr  const std::size_t SchemaSize = LogonSchema.size();        
    }; 

}