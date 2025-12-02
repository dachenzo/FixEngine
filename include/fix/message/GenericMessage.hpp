#pragma once
#include <vector>
#include <string>
#include <tuple>


namespace Fix::Message {

    struct GenericField {
        std::size_t tag;
        std::string value;
    };


    using GenericMessage = std::vector<GenericField>;
}