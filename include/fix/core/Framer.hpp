#pragma once
#include <cstddef>
#include <queue>
#include <cstdint>
#include <string_view>
#include <charconv>
#include <fix/core/LinearBuffer.hpp>

namespace Fix {

    enum class FramerContextState: std::uint8_t {
        FindingBegin,
        ReadingBody,
        Complete
    };

    struct FramerContext {
        std::uint64_t begin = 0;
        std::uint64_t end = 0;
        std::uint64_t body_len = 0;
        FramerContextState state = FramerContextState::FindingBegin;
    };
    
    

    struct Framer {
        static constexpr auto Fix_First_Field = "8=FIX.4.4\x01";
        
        static std::size_t start_size() {return Fix::LinearBuffer<unsigned char>::Start_Capacity;}
        Framer() = default;
        ~Framer() = default;
        Framer(const Framer&) = delete;
        Framer& operator=(const Framer&) = delete;
        Framer(Framer&&) = delete;  
        Framer& operator=(Framer&&) = delete;


        bool has_message() const noexcept;

        std::string_view get_message() const noexcept;

        void consume_message()  noexcept;

        void append(std::string_view data);

        private:
        bool parse_buffer();

        FramerContext current_context_;
        FramerContext cached_context_;
        LinearBuffer<unsigned char> buffer_;
        
        
        
    };
    
}