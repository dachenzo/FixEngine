#pragma once
#include <cstddef>
#include <vector>
#include <string>
#include "fix_message.hpp"



namespace Fix {

    struct Serializer {

        Serializer();

        void serialize(Fix::Message message);



        private:
        std::vector<char> buff_;

        

    };

};