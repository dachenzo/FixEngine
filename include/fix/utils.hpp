#pragma once 

#include <string_view>
#include <charconv>

#include <fix/definitions.hpp>


namespace Fix::Utils {
    #if defined(_WIN32)
        #include <windows.h>
        inline unsigned long get_pid() {
            return static_cast<unsigned long>(GetCurrentProcessId());
        }
    #else 
        #include <unistd.h>
        inline unsigned long get_pid() {
            return static_cast<unsigned long>(getpid());
        }
    #endif

    inline std::string_view role_to_string(Fix::Role role) noexcept;


    inline bool parse_int(std::string_view sv, std::size_t& out) noexcept {
        int tmp = 0;
            auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), tmp);
            if (ec != std::errc() || ptr != sv.data() + sv.size() || tmp < 0) {
                return false;
            }
            out = static_cast<std::size_t>(tmp);
            return true;
    };

}