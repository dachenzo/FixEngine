#include <cstddef>
#include <string_view>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <utility>
#include <charconv>
#include <vector>
#include "fix_message.hpp"

const size_t MAX_TAG_SIZE = 6;

namespace Fix {

    struct FixParseException: std::runtime_error {
        using std::runtime_error::runtime_error;
    };
    
    enum class ParseErrors {
        NoTag,
        MaxTagSize,
        MalformedTag,
        MissingEqualSign,
        MissingValue

    };


   

    struct Parser {

    
        std::optional<Fix::Message> parse(std::string_view& sv) {
            add_new_messge_fragment_(sv);
            parse_field_();
            if (message_builder.ready()) {return message_builder.get();}
            else {return std::nullopt;}
        }   

        


        private:
        std::vector<char> buff_;
        char tag_buff_[MAX_TAG_SIZE];
        size_t complete_field_count_{0};
        size_t read_idx_{0};
        Fix::MessageBuilder message_builder;
        std::vector<Fix::ParseErrors> errs_;


        std::string_view next_field_() {
            auto it = read_idx_;
            while(buff_[it] != '\x01') {
                it++;
            }
            read_idx_ = ++it;
            return std::string_view{buff_.data()+read_idx_, it-read_idx_};
        }

        void add_new_messge_fragment_(std::string_view& sv) {
            for (auto c: sv) {
                buff_.push_back(c);
                if (c == '\x01') {complete_field_count_++;}
            }
        }

        
        bool has_complete_field_() {
            return complete_field_count_ > 0;
        }

        void parse_field_() {
            
        
            if (!has_complete_field_()) {return ;}
           

            // read tag
            int idx = 0;
            std::string_view sv = next_field_();
            auto it = sv.begin();
            for (;it !=  sv.end() && *it != '='; it++) {
                if (idx == MAX_TAG_SIZE) {errs_.push_back(ParseErrors::MaxTagSize); break;}
                tag_buff_[idx] = *it;
                idx++;
                
            }

            int tag;
            auto [ptr, ec] = std::from_chars(tag_buff_, tag_buff_+idx, tag);
            std::string_view value_sv{it+1, sv.end()};
            

            if (ec != std::errc()) {errs_.push_back(ParseErrors::MalformedTag);}
            if (it ==  sv.end()) {errs_.push_back(ParseErrors::MissingValue);}
            else {
                if (*it != '=') {errs_.push_back(ParseErrors::MissingEqualSign);}
                // skip '='
                it++;
                if (it ==  sv.end()) {errs_.push_back(ParseErrors::MissingValue);}
            }
            
            Fix::Field field{tag, std::string{value_sv}, sv};

            if (errs_.empty()) {
                message_builder.add(field); 
            }
        }
    };  

};