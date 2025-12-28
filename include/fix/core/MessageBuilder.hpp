#pragma once
#include <cstdlib>
#include <span>
#include <fix/core/Message.hpp>
#include <fix/error/ParserErrors.hpp>
#include <fix/message/GenericMessage.hpp>
#include <fix/error/Severity.hpp>
#include <vector>



namespace Fix {

    struct RawField{
        int tag;
        std::string value;
        std::string_view raw_bytes; // WARNING MIGHT BE INVALID IF MISUSED
    };

    struct BuildResult {
        bool has_errors;
        bool ready;
    };

    struct MessageError {
        std::vector<Error::Parse> errs;
        Error::Severity sev = Error::Severity::NA;
    };

    struct MessageErrorView {
        std::span<const Error::Parse> errs;
        Error::Severity sev = Error::Severity::NA;
    };

    struct MessageBuilder {

        void add(Fix::RawField& field);

        BuildResult ready() const ;

        MessageErrorView get_error_state() const;

        void reset_state();

        Message::GenericMessage get();

        private:
        std::size_t body_length_ = 0;
        std::size_t body_length_count_ = 0;
        std::size_t checksum_count_ = 0;
        MessageError error_;
        Message::GenericMessage message_;
        bool ready_{false};

        
    };
}