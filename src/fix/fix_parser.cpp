#include <cstddef>
#include <string_view>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <utility>
#include <charconv>
#include <vector>
#include "fix_message.hpp"
#include "fix_parser.hpp"



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

    
        std::optional<Fix::Message> Parser::parse(std::string_view& sv) {
            add_new_messge_fragment_(sv);
            while (has_complete_field_()) {
                parse_field_();
            }
            if (message_builder.ready()) {return message_builder.get();}
            else {return std::nullopt;}
        }   

        

        std::string_view Parser::next_field_() {
            size_t start = read_idx_;
            // find SOH
            while (buff_[read_idx_] != '\x01') ++read_idx_;
            size_t end = read_idx_;
            // advance past SOH
            ++read_idx_;
            // now return [start..end)
            return std::string_view{ buff_.data() + start, end - start };
        }

        void Parser::add_new_messge_fragment_(std::string_view& sv) {
            for (auto c: sv) {
                buff_.push_back(c);
                if (c == '\x01') {complete_field_count_++;}
            }

            
        }

        
        bool Parser::has_complete_field_() {
            return complete_field_count_ > 0;
        }

        void Parser::parse_field_() {
            
        
            
            complete_field_count_ -= 1;

            // read tag
            int idx = 0;
            std::string_view sv = next_field_();
            
            
           
            auto it = sv.begin();
            for (;it !=  sv.end() && *it != '='; it++) {
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
                Fix::Field field{tag, std::string{value_sv}, sv};
                message_builder.add(field);  
            }
        }
    

};