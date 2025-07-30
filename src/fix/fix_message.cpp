#include <utility>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <unordered_map>
#include "fix_message.hpp"

namespace Fix {

    auto FIX_VERSION = "";

    
    std::optional<std::string_view> Message::get(int key) {
        auto it = lookup.find(key);
        if (it == lookup.end()) {
            return  std::nullopt;
        } else {
            return std::string_view{it->second};
        }
    }

    Message::Message() {
        message_.reserve(32);
        lookup.reserve(32);
    }

    void Message::add(Fix::Field& field) {
        message_.push_back(field);
        //FIXME: possible error if its a duplicate field
        auto it = lookup.insert({field.tag, field.value});
    }   

        

    namespace MessageErrors {
        enum class Critical {
            FAILED_CHECKSUM,
            WRONG_FIX_VERSION,
            WRONG_BODYLENGTH
        };
        enum class Mild {
            DUPLICATE_TAG
        };
    };
 
    void MessageBuilder::add(Fix::Field& field) {
        if (field.tag == 9) {
            body_length_ = std::stoi(field.value);
            body_length_count_ = 0;
        }

        if (field.tag == 10) {
            std::size_t checksum = std::stoi(field.value);
            if (checksum_count_ % 256 != checksum) {}
            if (body_length_count_ != body_length_) {}
            ready_ = true;
        }


        for (unsigned char c: field.raw_bytes) {checksum_count_ += c;}
        body_length_count_ += field.raw_bytes.size();
        message_.add(field);
    }

    bool MessageBuilder::ready() {return ready_;}

    Fix::Message MessageBuilder::get() {
        Fix::Message result = std::move(message_);
        reset_state_();
        return result;
    }

    

    void MessageBuilder::reset_state_() {
        message_ = Fix::Message();
        checksum_count_ = 0;
        body_length_count_ = 0;
        body_length_ = 0;
        ready_ = false;
    }


};