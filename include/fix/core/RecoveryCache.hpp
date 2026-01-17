#pragma once
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <cstring>
#include <limits>
#include <algorithm>
#include <array>
#include <fix/core/Message.hpp>




namespace Fix {

    struct MessageBounds {
        std::uint64_t offset;
        std::uint64_t size;

        bool operator==(const MessageBounds& other) const noexcept {
            return offset == other.offset && size == other.size;
        }

        bool operator!=(const MessageBounds& other) const noexcept {
            return !(*this == other);
        }
    };



    struct RecoveryCache {

        static constexpr std::uint64_t initial_capacity = 1024 * 64; // 64KB
        static constexpr std::uint64_t min_capacity_growth = 1024 * 32; // 32KB
        static constexpr std::uint64_t window = 2048; // cache [base_offset_ + window]
        static constexpr MessageBounds KEmpty{0, 0};
        static_assert(
            window <= std::numeric_limits<std::uint16_t>::max(), 
            "Window size exceeds uint16_t max value, cannot use uint16_t for slot size"
        );

        RecoveryCache();
        ~RecoveryCache();
        RecoveryCache(const RecoveryCache&) = delete;
        RecoveryCache& operator=(const RecoveryCache&) = delete;    
        RecoveryCache(RecoveryCache&&) = delete;
        RecoveryCache& operator=(RecoveryCache&&) = delete;

        bool inline empty() const noexcept;

        bool inline contains(SeqNum seqnum) const noexcept;

        std::string_view get(SeqNum seqnum);

        void consume(SeqNum seqnum);

        void start(SeqNum seqnum);

        void insert(SeqNum seqnum, std::string_view msg_wire);

        bool inline in_window(SeqNum seqnum) const noexcept;

        private:
        std::array<MessageBounds, window> slots_{};
        SeqNum base_offset_{0};
        std::byte* data_{nullptr};
        std::uint64_t buff_capacity_{0};
        std::uint64_t buff_size_{0};
        std::uint16_t size_{0};


        MessageBounds insert_message(std::string_view msg_wire);

        void reset();

        void grow_buffer(std::uint64_t extra_capacity);

        

        std::uint16_t inline slot_index(SeqNum seqnum) const noexcept;
    }; 
}