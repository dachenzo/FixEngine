#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <array>
#include <cstring>



namespace Fix {
    using Tag = std::uint32_t;

    struct MsgType {
        std::array<char, 8> value;
        std::uint8_t length;

        MsgType(std::string_view sv) {
            length = static_cast<std::uint8_t>(sv.size());
            if (sv.size() > value.size()) {
                length = static_cast<std::uint8_t>(value.size());
            }
            std::memcpy(value.data(), sv.data(), length);
        }

        MsgType(std::string& str) : MsgType(std::string_view(str)) {}

        MsgType(const char* cstr) : MsgType(std::string_view(cstr)) {}

        std::string_view to_string_view() const {
            return std::string_view(value.data(), length);
        }
    };

    struct GenericField {
        std::string value;
        Tag tag; 
    };

    struct GenericFieldView {
        std::string_view value;
        Tag tag;
    };

    template<typename T>
    using GenericMessage = std::vector<T>;
}