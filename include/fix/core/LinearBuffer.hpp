#pragma once
#include <cstddef>
#include <cassert>
#include <cstring>
#include <span>
#include <algorithm>


namespace Fix {
    template<typename T>
    struct LinearBuffer {
        static constexpr std::size_t Start_Capacity = 64 * 1024; // 64 KB
        static constexpr std::size_t min_grow_ = 16 * 1024; // 16 KB
        static_assert(Start_Capacity > 0, "Capacity must be greater than zero.");
        LinearBuffer() : base_offset_(0), capacity_(Start_Capacity), data_(new T[Start_Capacity]),  head_(0), tail_(0) {}


        ~LinearBuffer() {
            delete[] data_;
        }

        void push(std::span<cons T> items) {
            std::size_t needed_capacity = tail_ + items.size();
            if (needed_capacity > capacity_) {
                ensure_capacity(needed_capacity - capacity_);
            }
            std::memcpy(data_ + tail_, items.data(), items.size() * sizeof(T));
            tail_ += items.size();
        }

        std::size_t readable_rel_to_abs(std::size_t relative_offset) const noexcept {
            return base_offset_ + head_ + relative_offset;
        }

        std::size_t abs_to_readable_rel(std::size_t abs_offset) const noexcept {
            return abs_offset - base_offset_ - head_;
        }

        std::span<const T> data() const noexcept {
            return std::span<const T>(data_ + head_, tail_ - head_);
        }

        std::size_t head_offset() const noexcept {
            return head_;
        }

        std::size_t tail_offset() const noexcept {
            return tail_;
        }

        void consume(std::size_t count) noexcept {
            assert(count <= tail_ - head_);
            head_ += count;
            if (head_ == tail_) {
                // Reset buffer when all data is consumed
                base_offset_ += head_;
                head_ = 0;
                tail_ = 0;    
            }
        }

    
        private:
        std::size_t capacity_;
        std::uint64_t base_offset_;
        std::size_t head_;
        std::size_t tail_;
        T* data_;
        
        void ensure_capacity(std::size_t needed_capacity) {
            if (needed_capacity > capacity_ - tail_) {
                if (head_ > 0 && tail_ - head_ + needed_capacity <= capacity_) {
                    compact();
                } else {
                    grow(needed_capacity);
                }
            }
        }

        void grow(std::size_t min_increase) {
            std::size_t increase = std::max(min_increase, min_grow_);
            std::size_t new_capacity = capacity_ + increase;
            T* new_data = new T[new_capacity];
            std::memcpy(new_data, data_ + head_, (tail_ - head_) * sizeof(T));
            delete[] data_;
            data_ = new_data;
            tail_ -= head_;
            base_offset_ += head_;
            head_ = 0;
            capacity_ = new_capacity;
            
        }
        void compact() {
            std::memmove(data_, data_ + head_, (tail_ - head_) * sizeof(T));
            tail_ -= head_;
            base_offset_ += head_;
            head_ = 0;
        }
    };
}