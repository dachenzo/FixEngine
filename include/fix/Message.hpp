#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <span>
#include <optional>
#include <cstdlib>
#include <unordered_map>

namespace Fix {




    struct RawField{
        int tag;
        std::string value;
        std::string_view raw_bytes; // WARNING MIGHT BE INVALID IF MISUSED
    };

    struct Field {
        int tag;
        std::string value;
    };



    struct Message {
        Message();

        std::optional<std::string_view> get(int key) const;

        bool set_tag(int tag, std::string);

        void add(Fix::Field field);

        std::span<const Fix::Field> get_fields() const noexcept;


        std::span<const Fix::Field> get_fields_after(int tag) const;

        private:
        std::vector<Fix::Field> message_;
        std::unordered_map<int, std::size_t> lookup;
    };



    struct MessageBuilder {

        void add(Fix::RawField& field);

        bool ready();

        Fix::Message get();

        private:
        std::size_t body_length_ = 0;
        std::size_t body_length_count_ = 0;
        std::size_t checksum_count_ = 0;
        Fix::Message message_;
        bool ready_{false};

        void reset_state_();
    };

};