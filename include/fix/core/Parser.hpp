#pragma once

#include <cstddef>
#include <string_view>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <utility>
#include <charconv>
#include <vector>
#include <fix/core/MessageBuilder.hpp>
#include <fix/error/ParserErrors.hpp>
#include <fix/error/Severity.hpp>



namespace Fix {

    struct FixParseException: std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    struct ParseResult {
        std::vector<Error::Parse> errs;
        std::optional<Fix::Message> message; 
        Error::Severity sev;
    };
    
    
    constexpr const std::size_t DEFAULT_PARSER_BUFFER_SIZE = 1u << 12;
    const size_t MAX_TAG_SIZE = 10;

    struct Parser {
        Parser();
    
        ParseResult parse(std::string_view& sv);  

        
        private:
        std::vector<char> buff_;
        size_t complete_field_count_{0};
        size_t read_idx_{0};
        double compact_ratio_ = 0.25;
        Fix::MessageBuilder message_builder;
        std::vector<Fix::Error::Parse> errs_;
        
        std::string_view next_field_();

        void add_new_messge_fragment_(std::string_view& sv);

        bool has_complete_field_();

        void parse_field_();

        void maybe_compact_buffer_();

        std::size_t unread_() const noexcept;
    };  

};