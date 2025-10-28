#include <string>
#include <cstring>
#include <fix/Parser.hpp>
#include <fix/Message.hpp>

namespace Fix {
    std::string to_string(Fix::ParseErrors::Critical err) {
            switch (err)
            {
            case Fix::ParseErrors::Critical::MaxTagSize: return "MaxTagSize";
            case Fix::ParseErrors::Critical::NoTag: return "NoTag";
            case Fix::ParseErrors::Critical::MalformedTag: return "MalformedTag";
            case Fix::ParseErrors::Critical::MissingEqualSign: return "MissingEqualSign";
            case Fix::ParseErrors::Critical::MissingValue: return "MissingValue";
            default: return "Unknown Error";
                
            }
        }


    Parser::Parser() {
        buff_.reserve(D_PARSER_BUFFER_SIZE);
    }

    
    std::optional<Fix::Message> Parser::parse(std::string_view& sv) {
        add_new_messge_fragment_(sv);
        while (has_complete_field_()) {
            parse_field_();
        }

        maybe_compact_buffer_();

        if (message_builder.ready()) {return message_builder.get();}
        else {return std::nullopt;}
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

        // read tag
        int idx = 0;
        std::string_view sv = next_field_();
        
        
        
        auto it = sv.begin();
        for (;it != sv.end() && *it != '='; it++) {
            if (idx == MAX_TAG_SIZE) {errs_.push_back(ParseErrors::Critical::MaxTagSize); break;}
            tag_buff_[idx] = *it;
            idx++;
            
        }

        int tag;
        auto [ptr, ec] = std::from_chars(tag_buff_, tag_buff_+idx, tag);

        //skip '='1
        if (*it == '=') {it++;}
        else{errs_.push_back(ParseErrors::Critical::MissingEqualSign);}


        std::string_view value_sv = sv.substr(it - sv.begin());
        

        if (ec != std::errc()) {errs_.push_back(ParseErrors::Critical::MalformedTag);}
        if (it ==  sv.end()) {errs_.push_back(ParseErrors::Critical::MissingValue);}
        
        
        for (auto p: errs_) {std::cout << Fix::to_string(p) << '\n';}
        
        

        if (errs_.empty()) {
            Fix::RawField field{tag, std::string{value_sv}, sv};
            message_builder.add(field); 
            
        }
    }
    

}