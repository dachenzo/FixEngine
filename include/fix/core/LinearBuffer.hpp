#pragma once

#include <cstdint>
#include <cassert>
#include <cstring>
#include <span>
#include <algorithm>

namespace Fix {

    template <typename T>
    struct LinearBuffer {
        static_assert(std::is_trivially_copyable_v<T>);
        static_assert(std::is_trivially_destructible_v<T>);

        static constexpr std::uint64_t Start_Capacity = 64 * 1024; // 64 KB
        static constexpr std::uint64_t Min_Grow       = 16 * 1024; // 16 KB
        static_assert(Start_Capacity > 0);

        LinearBuffer()
            : capacity_(Start_Capacity),
            data_(new T[Start_Capacity]),
            head_(0),
            tail_(0),
            base_abs_(0) {}

        ~LinearBuffer() { delete[] data_; }

        LinearBuffer(const LinearBuffer&) = delete;
        LinearBuffer& operator=(const LinearBuffer&) = delete;
        LinearBuffer(LinearBuffer&&) = delete;
        LinearBuffer& operator=(LinearBuffer&&) = delete;

        // ---- Basic geometry ----
        std::uint64_t readable_size() const noexcept { return tail_ - head_; }
        bool empty() const noexcept { return tail_ == head_; }

        std::uint64_t base_abs() const noexcept { return base_abs_; }
        std::uint64_t head_local() const noexcept { return head_; }
        std::uint64_t tail_local() const noexcept { return tail_; }
        std::uint64_t capacity() const noexcept { return capacity_; }

        // ---- Views ----
        std::span<const T> readable() const noexcept {
            return { data_ + head_, tail_ - head_ };
        }

        std::span<T> readable() noexcept {
            return { data_ + head_, tail_ - head_ };
        }

        // ---- Absolute <-> local (local = index from data_[0]) ----
        std::uint64_t local_to_abs(std::uint64_t local) const noexcept {
            return base_abs_ + static_cast<std::uint64_t>(local);
        }

        std::uint64_t abs_to_local(std::uint64_t abs) const noexcept {
            assert(abs >= base_abs_);
            return static_cast<std::uint64_t>(abs - base_abs_);
        }

        // ---- Readable-relative convenience ----
        // readable-relative 0 == readable().data()[0] == data_[head_]
        std::uint64_t readable_rel_to_abs(std::uint64_t rel) const noexcept {
            return local_to_abs(head_ + rel);
        }

        std::uint64_t abs_to_readable_rel(std::uint64_t abs) const noexcept {
            std::uint64_t local = abs_to_local(abs);
            assert(local >= head_);
            return local - head_;
        }

        // ---- Mutation ----
        void push(std::span<const T> items) {
            ensure_writable(items.size());
            std::memcpy(data_ + tail_, items.data(), items.size() * sizeof(T));
            tail_ += items.size();
        }

        // Discard `count` bytes from the front of readable().
        void discard_prefix(std::uint64_t count) noexcept {
            assert(count <= (tail_ - head_));
            head_ += count;

            if (head_ == tail_) {
                // Buffer empty: advance base by the amount we discarded and reset indices.
                base_abs_ += static_cast<std::uint64_t>(head_);
                head_ = 0;
                tail_ = 0;
                return;
            }

            // Optional: compact opportunistically if we've left lots of slack.
            // (You can tune/remove this policy.)
            if (head_ >= capacity_ / 2) {
                compact();
            }
        }

        // Compact alive bytes [head_, tail_) -> [0, tail_-head_).
        void compact() noexcept {
            const std::uint64_t live = tail_ - head_;
            if (live == 0) {
                // If empty, normalize to fully reset.
                base_abs_ += static_cast<std::uint64_t>(head_);
                head_ = tail_ = 0;
                return; 
            }

            std::memmove(data_, data_ + head_, live * sizeof(T));
            base_abs_ += static_cast<std::uint64_t>(head_);
            head_ = 0;
            tail_ = live;
        }

        // ---- Clear ----
        void reset() noexcept {
        
            base_abs_ = 0; 
            head_ = 0;
            tail_ = 0;
        }

    private:
        std::uint64_t capacity_;
        T* data_;
        std::uint64_t head_;
        std::uint64_t tail_;
        std::uint64_t base_abs_;

        void ensure_writable(std::uint64_t n) {
            // Already enough contiguous space at the end.
            if (tail_ + n <= capacity_) return;

            const std::uint64_t live = tail_ - head_;

            // If compacting would create enough contiguous space, do that first.
            if (head_ > 0 && live + n <= capacity_) {
                compact();
                return;
            }

            grow(n); // grow knows how much we need
        }

        void grow(std::uint64_t n) {
            const std::uint64_t live = tail_ - head_;
            const std::uint64_t required = live + n;

            // Grow by at least max(n, MinGrow), but also guarantee >= required.
            const std::uint64_t increase = std::max(n, Min_Grow);
            std::uint64_t new_capacity = capacity_ + increase;
            if (new_capacity < required) new_capacity = required;

            T* new_data = new T[new_capacity];
            if (live > 0) {
                std::memcpy(new_data, data_ + head_, live * sizeof(T));
            }

            delete[] data_;
            data_ = new_data;
            capacity_ = new_capacity;

            // We copied alive bytes down to index 0, so shift base by head_.
            base_abs_ += static_cast<std::uint64_t>(head_);
            head_ = 0;
            tail_ = live;
        }

        
    };

} // namespace Fix
