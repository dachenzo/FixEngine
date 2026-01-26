#pragma once
#include <cstdint>
#include <cassert>


namespace Fix {

    enum class RingPolicy {
        Grow,
        Reject,
        Overwrite
    };

    template<typename T, std::uint64_t Capacity, std::uint64_t MinGrowSize, RingPolicy Policy = RingPolicy::Grow>
    struct Ring {
        static_assert(Capacity > 0, "Capacity must be greater than zero");
        static_assert(
            (Policy == RingPolicy::Grow && MinGrowSize > 0) || (Policy == RingPolicy::Overwrite && MinGrowSize == 0) || (Policy == RingPolicy::Reject && MinGrowSize == 0),
            "Invalid RingPolicy"
        );

        Ring()
            : head_(0),
              tail_(0),
              capacity_(Capacity),
              buffer_(new T[Capacity]) {}

        ~Ring() {
            delete[] buffer_;
        }

        Ring(const Ring&) = delete;
        Ring& operator=(const Ring&) = delete;
        Ring(Ring&&) = default;
        Ring& operator=(Ring&&) = default;

        bool empty() const noexcept {
            return size_ == 0;
        }

        bool full() const noexcept {
            return size_ == capacity_;
        }

        bool push(const T& item) {
            if (full()) {
                if constexpr (Policy == RingPolicy::Grow) {
                    const std::size_t new_capacity = capacity_ + MinGrowSize;
                    T* new_buffer = new T[new_capacity];
                    std::size_t j = 0;
                    for (std::uint64_t k = 0; k < size_; ++k) {
                        new_buffer[k] = buffer_[(head_ + k) % capacity_];
                    }
                    head_ = 0;
                    tail_ = size_;
                    delete[] buffer_;
                    buffer_ = new_buffer;
                    capacity_ = new_capacity;
                } else if constexpr (Policy == RingPolicy::Overwrite) {
                    head_ = next(head_);
                    --size_;
                }
                else if constexpr (Policy == RingPolicy::Reject) {
                    return false; // Do not add the new item
                }
            }
            buffer_[tail_] = item;
            tail_ = next(tail_);
            ++size_;
            return true;
        }

        const T& front() const {
            assert(!empty());
            return buffer_[head_];
        }

        const T& back() const {
            assert(!empty());
            std::uint64_t idx = prev(tail_);
            return buffer_[idx];
        }

        void pop_front() {
            assert(!empty());
            head_ = next(head_);
            --size_;
        }

        void pop_back() {
            assert(!empty());
            tail_ = prev(tail_);
            --size_;
        }

        void clear() noexcept {
            // loop through and destruct elements
            for (std::uint64_t i = 0; i < size_; i++) {
                buffer_[head_].~T();
                head_ = next(head_);
            }
            head_ = 0;
            tail_ = 0;
            size_ = 0;
        }

        std::uint64_t size() const noexcept {
            return size_;
        }
    
        

        private:
        std::uint64_t head_ = 0;
        std::uint64_t tail_ = 0;
        std::uint64_t capacity_ = Capacity;
        std::uint64_t size_ = 0;
        T* buffer_ = nullptr;

        inline std::uint64_t next(std::uint64_t idx) const noexcept {
            return (idx + 1) % capacity_;
        }

        inline std::uint64_t prev(std::uint64_t idx) const noexcept {
            return (idx == 0) ? capacity_ - 1 : idx - 1;
        }

    };
}