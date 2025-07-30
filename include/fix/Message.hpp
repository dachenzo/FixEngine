#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <unordered_map>

namespace Fix {


    struct Field{
        int tag;
        std::string value;
        std::string_view raw_bytes;
    };


    struct Message {
        std::optional<std::string_view> get(int key);

        void add(Fix::Field& field);

        private:
        std::vector<Fix::Field> message_;
        std::unordered_map<int, std::string> lookup;
    };



    struct MessageBuilder {

        void add(Fix::Field& field);

        bool ready();

        Fix::Message get();

        private:
        std::size_t body_length_;
        std::size_t body_length_count_;
        std::size_t checksum_count_;
        Fix::Message message_;
        
        bool ready_{false};

        void reset_state_();
    };

};