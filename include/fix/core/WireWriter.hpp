#pragma once 
#include <fix/core/Arena.hpp>
#include <stdexcept>
#include <string_view>

namespace Fix {
    
    struct WireWriter {

        WireWriter(ArenaHandle& handle);

        WireWriter(const WireWriter&) = delete;
        WireWriter& operator=(const WireWriter&) = delete;    
        WireWriter(WireWriter&&) = delete;
        WireWriter& operator=(WireWriter&&) = delete;

        void append(std::string_view data);
        void append(const char* data, std::size_t size);
        void append(char c);
        void append_eq();
        void append_soh();
        void append_int(long long value);

        std::string_view view() const noexcept;

        private:
        std::size_t size_ = 0;
        Fix::ArenaHandle& handle_;


    };
}