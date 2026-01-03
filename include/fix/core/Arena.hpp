#pragma once 
#include <vector>
#include <array>
#include <cstdint>
#include <cstring>

namespace Fix {
    struct Arena;

    enum class MemSourceType : std::uint8_t {
        Arena,
        Heap,
        None
    };

    struct ArenaHandle {
        friend struct Arena;
        ArenaHandle& operator=(ArenaHandle&) = delete;
        ArenaHandle(ArenaHandle&) = delete;

        ArenaHandle(ArenaHandle&& other) noexcept;

        ArenaHandle& operator=(ArenaHandle&& other) noexcept;

        std::byte* data() const noexcept;

        std::size_t capacity() const noexcept;

        MemSourceType source_type() const noexcept {
            return source_type_;
        }

        ~ArenaHandle();

        explicit operator bool() const noexcept;

        private:
        ArenaHandle() = default;
        Arena* arena_ = nullptr;
        std::byte* data_ptr_ = nullptr;
        std::size_t cap_ = 0;
        std::uint32_t index;
        MemSourceType source_type_ = MemSourceType::None;
    };

    struct Arena {
        static constexpr const std::size_t block_size = 1024; // 16 KB
        static constexpr const std::size_t block_count = 16;    // 16 blocks
        static constexpr const std::uint32_t end = UINT32_MAX;

        static_assert(block_count < end, "block_count must be less than end sentinel value");

        
        Arena (); 
        
        Arena(const Arena&) = delete;
        Arena& operator=(const Arena&) = delete;    
        Arena( Arena&&) = delete;
        Arena& operator=( Arena&&) = delete;

        ~Arena();

        ArenaHandle allocate(std::size_t size);

        std::byte* block_pointer(std::uint32_t index);

        



        private:
        friend struct ArenaHandle;
        std::byte* start;
        std::uint32_t free_index;

        ArenaHandle allocate_heap(std::size_t size);

        ArenaHandle allocate_arena();

        void release(uint32_t index); 

        void write_next(std::uint32_t idx, std::uint32_t next);

        std::uint32_t read_next(std::uint32_t idx);


    };
}