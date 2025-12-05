#pragma once
#include <array>
#include <fix/schema/Field.hpp>

namespace Fix::Message
{   
    using namespace Fix::Schema;

    inline constexpr std::array<FieldSchema, 3> RawGroup627Schema {{
        {628, FieldType::STRING, FieldPresence::OPTIONAL}, // HopCompID
        {629, FieldType::STRING, FieldPresence::OPTIONAL}, // HopSendingTime
        {630, FieldType::INT,    FieldPresence::OPTIONAL}, // HopRefID
    }};

    inline constexpr std::array<std::pair<int, std::size_t>, 3> Group627SchemaOccurrence {{
        {628, 0}, 
        {629, 0}, 
        {630, 0}, 
    }};

    inline constexpr GroupSchema Group627Schema {
        RawGroup627Schema.data(),
        RawGroup627Schema.size()
    };

    
    inline constexpr std::array<FieldSchema, 27> StandardHeaderSchema {{
        {8,   FieldType::STRING, FieldPresence::REQUIRED}, // BeginString
        {9,   FieldType::INT,    FieldPresence::REQUIRED}, // BodyLength
        {35,  FieldType::STRING, FieldPresence::REQUIRED}, // MsgType
        {49,  FieldType::STRING, FieldPresence::REQUIRED}, // SenderCompID
        {56,  FieldType::STRING, FieldPresence::REQUIRED}, // TargetCompID
        {34,  FieldType::INT,    FieldPresence::REQUIRED}, // MsgSeqNum
        {52,  FieldType::STRING, FieldPresence::REQUIRED}, // SendingTime

        {115, FieldType::STRING, FieldPresence::OPTIONAL}, // OnBehalfOfCompID
        {128, FieldType::STRING, FieldPresence::OPTIONAL}, // DeliverToCompID

        {90,  FieldType::INT,    FieldPresence::OPTIONAL}, // SecureDataLen
        {91,  FieldType::STRING, FieldPresence::OPTIONAL}, // SecureData

        

        {50,  FieldType::STRING, FieldPresence::OPTIONAL}, // SenderSubID
        {142, FieldType::STRING, FieldPresence::OPTIONAL}, // SenderLocationID

        {57,  FieldType::STRING, FieldPresence::OPTIONAL}, // TargetSubID
        {143, FieldType::STRING, FieldPresence::OPTIONAL}, // TargetLocationID

        {116, FieldType::STRING, FieldPresence::OPTIONAL}, // OnBehalfOfSubID
        {144, FieldType::STRING, FieldPresence::OPTIONAL}, // OnBehalfOfLocationID

        {129, FieldType::STRING, FieldPresence::OPTIONAL}, // DeliverToSubID
        {145, FieldType::STRING, FieldPresence::OPTIONAL}, // DeliverToLocationID

        {43,  FieldType::STRING, FieldPresence::OPTIONAL}, // PossDupFlag
        {97,  FieldType::BOOLEAN, FieldPresence::OPTIONAL}, // PossResend

        
        {122, FieldType::STRING, FieldPresence::OPTIONAL}, // OrigSendingTime

        {212, FieldType::INT,    FieldPresence::OPTIONAL}, // XmlDataLen
        {213, FieldType::STRING, FieldPresence::OPTIONAL}, // XmlData

        {347, FieldType::STRING, FieldPresence::OPTIONAL}, // MessageEncoding
        {369, FieldType::INT,    FieldPresence::OPTIONAL}, // LastMsgSeqNumProcessed

        {627, FieldType::GROUP,    FieldPresence::OPTIONAL, &Group627Schema}// NoHops
        
    }};
   

} // namespace Fix::Message
