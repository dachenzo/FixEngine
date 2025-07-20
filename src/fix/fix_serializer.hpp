#include <cstddef>
#include <vector>
#include <string>
#include "fix_message.hpp"



namespace Fix {

    struct Serializer {

        Serializer(): buff_{} {
            buff_.reserve(8192);
        }

        void serialize(Fix::Message message) {
            for (Fix::Field& field: message) {
                std::string tag = std::to_string(field.tag);
                for (auto& c: tag) {buff_.push_back(c);}
                buff_.push_back('=');
                for (auto& c: field.value) {buff_.push_back(c);}
                buff_.push_back('\x01');
            }
        }



        private:
        std::vector<char> buff_;

        

    };

};