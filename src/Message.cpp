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


    bool Message::add(Fix::Field field) {
        auto it = lookup.insert({field.tag, message_.size()});
        if (!it.second) {
            return false;
        }
        message_.push_back(field);
        return true;
       
        

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

        

};

