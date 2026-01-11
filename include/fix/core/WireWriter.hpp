#pragma once 
#include <fix/core/Arena.hpp>
#include <stdexcept>
#include <charconv>
#include <string_view>

namespace Fix {
    
    struct WireWriter {
        static WireWriter from_arena(Fix::Arena& arena, std::string_view msg_wire);

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
        void append_int(int64_t value);
        std::byte* data() const noexcept;
        std::size_t size() const noexcept;

        std::string_view view() const noexcept;

        private:
        Fix::ArenaHandle handle_;
        char int_buff_[22]; // enough for int64_t
        std::size_t size_ = 0;
        


    };
}