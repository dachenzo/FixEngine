#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <span>
#include <optional>
#include <cstdlib>
#include <unordered_map>

namespace Fix {




    

    struct Field {
        int tag;
        std::string value;
    };



    struct Message {
        Message();

        std::optional<std::string_view> get(int key) const;

        bool set_tag(int tag, std::string);

        bool add(Fix::Field field);

        std::span<const Fix::Field> get_fields() const noexcept;


        std::span<const Fix::Field> get_fields_after(int tag) const;

        private:
        std::vector<Fix::Field> message_;
        std::unordered_map<int, std::size_t> lookup;
    };



};