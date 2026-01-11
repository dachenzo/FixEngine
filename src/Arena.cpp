#include <fix/core/Arena.hpp>


namespace Fix {


    ArenaHandle& ArenaHandle::operator=(ArenaHandle&& other) noexcept {
        if (this != &other) {
            if (source_type_ == MemSourceType::Arena && arena_) {
                arena_->release(index);
            } 
            if (source_type_ == MemSourceType::Heap && data_ptr_) {
                ::operator delete(data_ptr_);
            }

            arena_ = other.arena_;
            index = other.index;
            data_ptr_ = other.data_ptr_;
            cap_ = other.cap_;
            source_type_ = other.source_type_;
            other.arena_ = nullptr;
            other.index = UINT32_MAX;
            other.data_ptr_ = nullptr;
            other.source_type_ = MemSourceType::None;
            other.cap_ = 0;
        }
        return *this;
    }

    ArenaHandle::ArenaHandle(ArenaHandle&& other) noexcept
        : arena_(other.arena_),
          index(other.index),
          data_ptr_(other.data_ptr_),
          cap_(other.cap_),
          source_type_(other.source_type_) {
        other.arena_ = nullptr;
        other.index = UINT32_MAX;
        other.data_ptr_ = nullptr;
        other.source_type_ = MemSourceType::None;
        other.cap_ = 0;
 
    }

    std::byte* ArenaHandle::data() const noexcept {
        return data_ptr_;
    }

    std::size_t ArenaHandle::capacity() const noexcept {
        return cap_;
    }


    ArenaHandle::~ArenaHandle() {
        if (source_type_ == MemSourceType::Heap && data_ptr_) {
            ::operator delete(data_ptr_);
        } else if (source_type_ == MemSourceType::Arena && arena_) {
            arena_->release(index);
        }
    }

    ArenaHandle::operator bool() const noexcept {
        return data_ptr_ != nullptr;
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

    ArenaHandle Arena::allocate_heap(std::size_t size) {
        std::byte* ptr = static_cast<std::byte*>(::operator new(size));
        ArenaHandle handle;
        handle.arena_ = nullptr;
        handle.data_ptr_ = ptr;
        handle.cap_ = size;
        handle.index = UINT32_MAX;
        handle.source_type_ = MemSourceType::Heap;
        return handle;
    }

    ArenaHandle Arena::allocate_arena() {
        std::uint32_t allocated_index = free_index;
        free_index = read_next(allocated_index);

        ArenaHandle handle;
        handle.arena_ = this;
        handle.data_ptr_ = block_pointer(allocated_index);
        handle.cap_ = block_size;
        handle.index = allocated_index;
        handle.source_type_ = MemSourceType::Arena;
        return handle;
    }

    ArenaHandle Arena::allocate(std::size_t size) {
        if (size > block_size || free_index == end) {
            return allocate_heap(size);
        }
        return allocate_arena();
    }




     
} // namespace Fix