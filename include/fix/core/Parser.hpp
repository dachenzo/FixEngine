#pragma once

#include <cstddef>
#include <string_view>
#include <stdexcept>
#include <vector>
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

    struct ParserContext {
        static constexpr const std::size_t MessageSizeReserve = 1024;
        static constexpr const std::size_t MessageErrorReserve = 16;
        GenericMessage<GenericFieldView> out_msg; std::vector<ParseErrorInfo> out_errs;

        ParserContext() {
            out_msg.reserve(MessageSizeReserve);
            out_errs.reserve(MessageErrorReserve);
        }   
    };




    struct Parser {
        static constexpr const std::size_t MAX_TAG_SIZE = 7;
        static constexpr const int32_t undefined_tag = 0;
        void parse(std::string_view msg_frame, GenericMessage<GenericFieldView>& out_msg, std::vector<ParseErrorInfo>& out_errs);

        private:

        bool parse_tag(std::string_view str, Tag& out_val);

        
    };
        

};