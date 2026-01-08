#pragma once 
#include <fix/core/Arena.hpp>
#include <stdexcept>
#include <string_view>

namespace Fix {
    
    struct WireWriter {

        WireWriter(ArenaHandle&& handle);

        WireWriter(const WireWriter&) = delete;
        WireWriter& operator=(const WireWriter&) = delete;    
        WireWriter(WireWriter&&) = default;
        WireWriter& operator=(WireWriter&&) = default;

        void append(std::string_view data);
        void append(const char* data, std::size_t size);
        void append(char c);
        void append_eq();
        void append_soh();
        void append_int(long long value);
        std::byte* data() const noexcept {
            return handle_.data();
        }
        std::size_t size() const noexcept {
            return size_;
        }

        std::string_view view() const noexcept;

        private:
        Fix::ArenaHandle handle_;
        std::size_t size_ = 0;
        


    };
}