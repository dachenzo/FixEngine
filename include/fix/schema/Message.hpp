#pragma once

#include <concepts>

#include <fix/schema/Field.hpp>
#include <fix/message/Header.hpp>
#include <fix/message/Trailer.hpp>

namespace Fix::Message {

    template<typename T>
    concept MessageBody = requires {
        { T::MsgType } -> std::convertible_to<const std::string&>;
        { T::Schema } -> std::convertible_to<const Fix::Schema::FieldSchema*>;
        { T::SchemaSize } -> std::convertible_to<std::size_t>;
    };

    template<MessageBody Body>
    struct MessageSchema {
        const std::string msg_type = Body::MsgType;
        const Fix::Schema::FieldSchema* header = StandardHeaderSchema.data();
        const std::size_t header_field_count = StandardHeaderSchema.size();

        const Fix::Schema::FieldSchema* body = Body::Schema;
        const std::size_t body_field_count = Body::SchemaSize;

        const Fix::Schema::FieldSchema* trailer = TrailerSchema.data();
        const std::size_t trailer_field_count = TrailerSchema.size();
    };

    
}