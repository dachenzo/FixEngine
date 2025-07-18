#include <cstddef>
#include <string_view>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <utility>
#include <charconv>
#include <vector>
#include "fix_message.hpp"

constexpr const size_t MAX_MESSAGE_SIZE = 4096*2;
const size_t MAX_TAG_SIZE = 6;

namespace Fix {

    struct FixParseException: std::runtime_error {
        using std::runtime_error::runtime_error;
    };
    
    enum class ParseErrors {
        None,
        NoTag,
        MaxTagSize,
        MalformedTag,
        MissingEqualSign,
        MissingValue

    };


    using ParseResult = std::pair<std::optional<Fix::Field>, std::vector<ParseErrors>>;

    struct Parser {

        void add_new_messge_fragment(std::string_view sv) {
            for (auto c: sv) {
                if (write_idx_== MAX_MESSAGE_SIZE) {}//DO SOMETHING
                buff_[write_idx_] = c;
                write_idx_++;
                if (c == 1) {complete_message_count_++;}
            }
        }

        

        Fix::ParseResult parse_field() {
            std::vector<Fix::ParseErrors> errs;
        
            if (!has_complete_message) {std::cout << "Wrong use, no theres no message yet";}
            ParseErrors err = ParseErrors::None;

            // read tag
            int idx = 0;
            std::string_view sv = next_field_();
            auto it = sv.begin();
            for (;it !=  sv.end() && *it != '='; it++) {
                if (idx == MAX_TAG_SIZE) {errs.push_back(ParseErrors::MaxTagSize); break;}
                tag_buff_[idx] = *it;
                idx++;
                
            }

            int tag;
            auto [ptr, ec] = std::from_chars(tag_buff_, tag_buff_+idx, tag);
            std::string_view value_sv{it+1, sv.end()};
            

            if (ec != std::errc()) {errs.push_back(ParseErrors::MalformedTag);}
            if (it ==  sv.end()) {errs.push_back(ParseErrors::MissingValue);}
            else {
                if (*it != '=') {errs.push_back(ParseErrors::MissingEqualSign);}
                // skip '='
                it++;
                if (it ==  sv.end()) {errs.push_back(ParseErrors::MissingValue);}
            }
            
            Fix::Field field{tag, std::string{value_sv}};
            if (errs.empty()) {
                return ParseResult{field, errs};
            } else {
                return ParseResult{std::nullopt, errs};
            }

        }

        bool has_complete_message() {
            return complete_message_count_ > 0;
        }



        private:
        char buff_[MAX_MESSAGE_SIZE];
        char tag_buff_[MAX_TAG_SIZE];
        size_t complete_message_count_{0};
        size_t write_idx_{0};
        size_t read_idx_{0};


        std::string_view next_field_() {
            auto it = read_idx_;
            while(buff_[it] != 1) {
                it++;
            }
            return std::string_view{buff_+read_idx_, it-read_idx_};
        }
    };


};