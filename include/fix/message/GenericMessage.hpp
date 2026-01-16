#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <tuple>


namespace Fix {
    using Tag = std::uint32_t;

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