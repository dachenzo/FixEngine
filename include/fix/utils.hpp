#pragma once 

#include <fix/definitions.hpp>
#include <string_view>

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


    inline bool parse_int(std::string_view sv, std::size_t& out) noexcept;

}