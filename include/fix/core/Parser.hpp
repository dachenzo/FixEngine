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
#include <fix/message/GenericMessage.hpp>
#include <fix/error/ParserErrors.hpp>
#include <fix/error/Severity.hpp>



namespace Fix {

    struct FixParseException: std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    struct ParseErrorInfo {
        Tag tag;
        ParseError code;
    };




    struct Parser {
        static constexpr const std::size_t MAX_TAG_SIZE = 7;
        static constexpr const int32_t undefined_tag = 0;
        void parse(std::string_view msg_frame, GenericMessage<GenericFieldView>& out_msg, std::vector<ParseErrorInfo>& out_errs);

        private:

        bool parse_tag(std::string_view str, Tag& out_val);

        
    };
        

};