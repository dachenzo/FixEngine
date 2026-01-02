#include <fix/core/Arena.hpp>


namespace Fix {

    ArenaHandle::ArenaHandle(ArenaHandle&& other) noexcept
        : arena(other.arena), index(other.index) {
        other.arena = nullptr;
        other.index = UINT32_MAX;
    }

    ArenaHandle& ArenaHandle::operator=(ArenaHandle&& other) noexcept {
        if (this != &other) {
            if (arena) {
                arena->release(index);
            }
            arena = other.arena;
            index = other.index;
            other.arena = nullptr;
            other.index = UINT32_MAX;
        }
        return *this;
    }

    std::byte* ArenaHandle::data() const {
        if (arena) {
            return arena->block_pointer(index);
        }
        return nullptr;
    }

    std::uint32_t ArenaHandle::size() {
        if (arena) {
            return Arena::block_size;
        }
        return 0;
    }

    ArenaHandle::~ArenaHandle() {
        if (arena) {
            arena->release(index);
        }
    }

    Arena::Arena () {
        start = static_cast<std::byte*>(::operator new(block_size * block_count));
        free_index = 0;
        for (std::uint32_t i = 0; i < block_count; ++i) {
            write_next(i, i + 1);
        }
        write_next(block_count - 1, end);
    }


    Arena::~Arena() {
        ::operator delete(start);
    }

    std::byte* Arena::block_pointer(std::uint32_t index) {
        return start + (index * block_size);
    }

    void Arena::release(uint32_t index) {
        write_next(index, free_index);
        free_index = index;
    }

    void Arena::write_next(std::uint32_t idx, std::uint32_t next) {
        std::byte* ptr = block_pointer(idx);
        std::memcpy(ptr, &next, sizeof(next));
    }

    std::uint32_t Arena::read_next(std::uint32_t idx) {
        std::byte* ptr = block_pointer(idx);
        std::uint32_t next;
        std::memcpy(&next, ptr, sizeof(next));
        return next;
    }

    ArenaHandle Arena::allocate(std::size_t size) {
        if (size > block_size || free_index == end) {
            return ArenaHandle{nullptr, UINT32_MAX};
        }
        std::uint32_t allocated_index = free_index;
        free_index = read_next(allocated_index);
        return ArenaHandle{this, allocated_index};
    }




     
} // namespace Fix