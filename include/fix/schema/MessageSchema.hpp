#pragma once

#include <concepts>
#include <string_view>

#include <fix/schema/Field.hpp>
#include <fix/message/Header.hpp>
#include <fix/message/Trailer.hpp>

namespace Fix::Schema {

    template<typename T>
    concept MessageBody = requires {
        { T::MsgType } -> std::convertible_to<std::string_view>;
        { T::Schema } -> std::convertible_to<const Fix::Schema::FieldSchema*>;
        { T::SchemaSize } -> std::convertible_to<const std::size_t>;
    };

   
    struct MessageSchema {
        std::string_view msg_type;
        const Fix::Schema::FieldSchema* body;
        const std::size_t body_field_count;
    };

    

    
}