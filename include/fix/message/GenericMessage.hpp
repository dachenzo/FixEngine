#pragma once
#include <vector>
#include <string>
#include <tuple>


namespace Fix::Message {

    struct GenericField {
        unsigned long tag;
        std::string value;
    };


    using GenericMessage = std::vector<GenericField>;
}