#include <array>
#include <fix/schema/Field.hpp>


namespace Fix::Message {
    using namespace Fix::Schema;


    inline constexpr std::array<FieldSchema, 3> TrailerSchema {{
        {10, FieldType::INT, FieldPresence::REQUIRED}, // CheckSum
        {83, FieldType::STRING, FieldPresence::OPTIONAL}, // Signature
        {93, FieldType::INT,    FieldPresence::OPTIONAL}  // SignatureLength
    }};
}