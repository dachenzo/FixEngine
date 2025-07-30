#pragma once

#include <cstddef>
#include <string_view>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <utility>
#include <charconv>
#include <vector>
#include "include/fix/Message.hpp"

const size_t MAX_TAG_SIZE = 6;

namespace Fix {

    struct FixParseException: std::runtime_error {
        using std::runtime_error::runtime_error;
    };
    
    struct ParseErrors {
        enum class Critical {
            NoTag,
            MaxTagSize,
            MalformedTag,
            MissingEqualSign,
            MissingValue
        };
    };


    struct Parser {

    
        std::optional<Fix::Message> parse(std::string_view& sv);  

        
        private:
        std::vector<char> buff_;
        char tag_buff_[MAX_TAG_SIZE];
        size_t complete_field_count_{0};
        size_t read_idx_{0};
        Fix::MessageBuilder message_builder;
        std::vector<Fix::ParseErrors::Critical> errs_;
        std::string_view next_field_();

        void add_new_messge_fragment_(std::string_view& sv);

        
        bool has_complete_field_();

        void parse_field_();
    };  

};