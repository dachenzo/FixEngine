#include <utility>
#include <string>
#include <string_view>
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
    
    std::optional<std::string_view> Message::get(int key) const {
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
        
        if (it == lookup.end() || ++it == lookup.end()) {// throw error
        }
        return std::span{message_.begin(), message_.end()}.subspan(it->second);
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
        if (raw_field.tag == 9) {
            body_length_ = std::stoi(raw_field.value);
            body_length_count_ = 0;
        }

        if (raw_field.tag == 10) {
            std::size_t checksum = std::stoi(raw_field.value);
            if (checksum_count_ % 256 != checksum) {}
            if (body_length_count_ != body_length_) {}
            ready_ = true;
        }


        for (unsigned char c: raw_field.raw_bytes) {checksum_count_ += c;}
        body_length_count_ += raw_field.raw_bytes.size();
        Fix::Field field{raw_field.tag, std::move(raw_field.value)};
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

