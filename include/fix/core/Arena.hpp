#pragma once 
#include <vector>
#include <array>
#include <cstdint>
#include <cstring>

namespace Fix {

    struct ArenaHandle {
        
        ArenaHandle(Arena* arena, std::uint32_t index) : arena(arena), index(index) {}

        ArenaHandle& operator=(ArenaHandle&) = delete;
        ArenaHandle(ArenaHandle&) = delete;

        ArenaHandle(ArenaHandle&& other) noexcept;

        ArenaHandle& operator=(ArenaHandle&& other) noexcept;

        std::byte* data() const;

        std::uint32_t size() const {
            return Arena::block_size;
        }

        ~ArenaHandle();

        explicit operator bool() const { return arena != nullptr; }

        private:
        Arena* arena;
        std::uint32_t index;
    };

    struct Arena {
        static constexpr const std::size_t block_size = 1024; // 16 KB
        static constexpr const std::size_t block_count = 16;    // 16 blocks
        
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
        std::uint32_t end = UINT32_MAX;

        void release(uint32_t index); 

        void write_next(std::uint32_t idx, std::uint32_t next);

        std::uint32_t read_next(std::uint32_t idx);


    };
}