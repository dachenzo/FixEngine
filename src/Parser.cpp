#include <string>
#include <cstring>
#include <fix/Parser.hpp>
#include <fix/Message.hpp>

namespace Fix {
    

    Parser::Parser() {
        buff_.reserve(DEFAULT_PARSER_BUFFER_SIZE);
    }

    
    ParseResult Parser::parse(std::string_view& sv) {
        add_new_messge_fragment_(sv);
        while (has_complete_field_() && !has_errors_()) {
            parse_field_();
        }

        maybe_compact_buffer_();
        auto build_result = message_builder.ready();
        if (!build_result.has_errors && build_result.ready) {
            return {{}, message_builder.get(), Error::Severity::NA};
        }
        else if (build_result.has_errors) {
            auto builder_errs = message_builder.get_error_state();
            Error::Severity error_severity = errs_.empty() ? builder_errs.sev : Error::Severity::Fatal;
            errs_.insert(errs_.end(), builder_errs.errs.begin(), builder_errs.errs.end());        
            return {errs_, {}, error_severity};
        }
        else {
            return {{}, {}, Error::Severity::NA};
        }
    }   

        

    std::string_view Parser::next_field_() {
        size_t start = read_idx_;
        // find SOH
        while (read_idx_ < buff_.size() && buff_[read_idx_] != '\x01') {++read_idx_;}
        if (read_idx_ == buff_.size()) {
            // No SOH found — caller must ensure there's a complete field before calling
            throw FixParseException{"Internal parser state: expected complete field but none found"};
        }
        // advance past SOH
        size_t end = read_idx_++;
        
     
        // now return [start..end)
        return std::string_view{buff_.data() + start, end - start };
    }

    void Parser::add_new_messge_fragment_(std::string_view& sv) {
        for (auto c: sv) {
            buff_.push_back(c);
            if (c == '\x01') {complete_field_count_++;}
        }

        
    }

    std::size_t Parser::unread_() const noexcept {
        return buff_.size() - read_idx_;
    }

    void Parser::maybe_compact_buffer_() {
        std::size_t unread = unread_();
       
        if (unread == 0) {
            read_idx_ = 0;
            buff_.clear();
            return;
        }
        

        std::size_t sz = buff_.size();
        
        if (read_idx_ >= static_cast<std::size_t>(sz*compact_ratio_)) {
             
            std::memmove(buff_.data(), buff_.data()+read_idx_, unread);
            buff_.resize(unread);
            read_idx_ = 0;
        }
    }

    
    bool Parser::has_complete_field_() {
        return complete_field_count_ > 0;
    }

    void Parser::parse_field_() {
        
        errs_.clear();
        complete_field_count_ -= 1;

        std::string_view sv = next_field_();   // "tag=value"

        // 1) Find '='
        auto eq_pos = sv.find('=');
        if (eq_pos == std::string_view::npos) {
            errs_.push_back(Error::Parse::MalformedTag);
            errs_.push_back(Error::Parse::NoTag);   // if you want to use this one too
            return;
        }
 
        // 2) Tag part
        auto tag_len = eq_pos;
        if (tag_len == 0) {
            errs_.push_back(Error::Parse::NoTag);
        } else if (tag_len > MAX_TAG_SIZE) {
            errs_.push_back(Error::Parse::MaxTagSize);
            tag_len = MAX_TAG_SIZE; // truncate for parsing, still error out
        }

        int tag = 0;
        if (tag_len > 0) {
            std::string_view tag_sv = sv.substr(0, tag_len);
            auto [ptr, ec] = std::from_chars(tag_sv.data(),
                                            tag_sv.data() + tag_sv.size(),
                                            tag);
            if (ec != std::errc()) {
                errs_.push_back(Error::Parse::MalformedTag);
            }
        }

        // 3) Value part
        std::string_view value_sv;
        if (eq_pos + 1 <= sv.size()) {
            value_sv = sv.substr(eq_pos + 1);
        }

        if (value_sv.empty()) {
            errs_.push_back(Error::Parse::MissingValue);
        }

        for (auto p: errs_) {
            std::cout << Fix::Error::to_string(p) << '\n';
        }

        if (errs_.empty()) {
            Fix::RawField field{tag, std::string{value_sv}, sv};
            message_builder.add(field);
        }
    }

    bool Parser::has_errors_() {
        return !errs_.empty();
    }
    

}