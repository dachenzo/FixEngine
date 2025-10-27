#include <utility>
#include <string>
#include <string_view>
#include <iostream>
#include <vector>
#include <optional>
#include <span>
#include <unordered_map>
#include <fix/Message.hpp>


namespace Fix {

    auto FIX_VERSION = "";

    Message::Message() {
        message_.reserve(32);
        lookup.reserve(32);
    }
    
    std::optional<std::string_view> Message::get(int key) const  {
        auto it = lookup.find(key);
        if (it == lookup.end()) {
            return  std::nullopt;
        } else {
            return std::string_view{message_[it->second].value};
        }
    }

    bool Message::set_tag(int tag, std::string value) {
        auto it = lookup.find(tag);
        if (it == lookup.end()) {return false;}

        message_[it->second].value = value;
        return true;
    }


    void Message::add(Fix::Field field) {
        message_.push_back(field);
        //FIXME: possible error if its a duplicate field
        auto it = lookup.insert({field.tag, message_.size()-1});
    }   

    std::span<const Fix::Field> Message::get_fields()const noexcept {
        return std::span{message_.begin(), message_.end()};
    }

    std::span<const Fix::Field> Message::get_fields_after(int tag)const {
        auto it = lookup.find(tag);
        if (it == lookup.end()) {
            // empty span
            return std::span<const Fix::Field>{message_.data() + message_.size(), 0};
        }
        std::size_t idx = it->second + 1;       // start *after* the tag
        if (idx > message_.size()) {
            return std::span<const Fix::Field>{message_.data() + message_.size(), 0};
        }
        return std::span<const Fix::Field>{message_.data() + idx, message_.size() - idx};
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
 
    void MessageBuilder::add(Fix::RawField& raw_field) {
        // Update checksum with *previous* bytes only; do not include tag 10.
        // For any field except 10, add raw bytes to checksum.
        if (raw_field.tag != 10) {
            for (unsigned char c: raw_field.raw_bytes) { checksum_count_ += c;  }
            checksum_count_ += static_cast<unsigned char>('\x01');
        
        }

        // Handle BodyLength
        if (raw_field.tag == 9) {
            body_length_ = std::stoi(raw_field.value);
            body_length_count_ = 0; // start counting *after* 9-field
        } else if (raw_field.tag != 10) {
            body_length_count_ += raw_field.raw_bytes.size()+1; // count everything between 9 and 10
        }

        // Handle CheckSum
        if (raw_field.tag == 10) {
            const std::size_t checksum = std::stoi(raw_field.value);
            const bool checksum_ok = (checksum_count_ % 256) == checksum;
            const bool body_ok  = (body_length_count_ == body_length_);
            ready_ = checksum_ok && body_ok;  
             // <-- only ready if both pass
            
        }

        // Store field in the message
        message_.add(Fix::Field{raw_field.tag, std::move(raw_field.value)});
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

