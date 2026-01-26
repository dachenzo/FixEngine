#pragma once
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <fix/core/LinearBuffer.hpp>
#include <fix/core/Ring.hpp>

namespace Fix {



    enum class FramerContextState: std::uint8_t {
        FindingBegin,
        ReadingBody,
        Complete
    };

    enum class FramerParseResult: std::uint8_t {
        QueuedMessage,   // pushed one message into completed_messages_
        ProgressNoMsg,   // advanced scan/discarded/resynced; should continue looping
        NeedMoreData     // cannot progress without more bytes
    };

    struct MessageWindow {
        std::uint64_t begin = 0;
        std::uint64_t end = 0;
    };

    struct FramerContext {
        std::uint64_t begin = 0;
        std::uint64_t end = 0;
        std::uint64_t body_len = 0;
        FramerContextState state = FramerContextState::FindingBegin;
    };



    inline MessageWindow make_message_window(const FramerContext& ctx) {
        return MessageWindow{ctx.begin, ctx.end};
    }
    
    

    struct Framer {
        static constexpr std::string_view Fix_First_Field = "8=FIX.4.4\x01";
        static constexpr std::uint64_t MAX_MESSAGE_SIZE = 1024*1024; // 1 MB
        static constexpr std::uint64_t MAX_BEGIN_TO_BODYLEN_FIELD_BYTES = 32; // should be enough to include TAG 8 AND 9
        
        static std::size_t start_size() {return Fix::LinearBuffer<unsigned char>::Start_Capacity;}
        Framer() = default;
        ~Framer() = default;
        Framer(const Framer&) = delete;
        Framer& operator=(const Framer&) = delete;
        Framer(Framer&&) = delete;  
        Framer& operator=(Framer&&) = delete;


        bool inline has_message() const noexcept {
            return completed_messages_.size() > 0;
        }

        std::string_view get_message() const noexcept;

        void consume_message()  ;

        void append(std::string_view data);

        void reset();

        private:
        FramerParseResult parse_buffer();

        void discard_n_from_head(std::uint64_t count);

        FramerContext current_context_;
        LinearBuffer<unsigned char> buffer_;
        Ring<MessageWindow, 1024, 0, RingPolicy::Reject> completed_messages_;
        std::uint64_t scan_abs_ = 0;
        
    };
    
}