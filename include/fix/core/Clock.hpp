#pragma once
#include <string>
#include <cstdint>
#include <concepts>

namespace Fix {

    template<typename T> 
    concept ClockLike = requires (T clock) {
        { clock.now_fix() } -> std::convertible_to<std::string>;
        {clock.now()} -> std::convertible_to<std::uint64_t>;
    };

    struct Clock {
        std::string now_fix();
        std::uint64_t now();
    };

}