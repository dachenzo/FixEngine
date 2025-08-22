#pragma once
#include <string>
#include <cstdlib>
#include <fix/Message.hpp>



namespace Fix {

    struct Serializer {
        std::size_t serialize(Fix::Message& msg, std::string& buff);
    };

}