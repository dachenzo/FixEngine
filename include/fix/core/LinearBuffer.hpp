#pragma once
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <cstring>
#include <span>
#include <algorithm>

namespace Fix {

    template <typename T>
    struct LinearBuffer {
        static constexpr std::size_t StartCapacity = 64 * 1024; // 64 KB
        static constexpr std::size_t MinGrow       = 16 * 1024; // 16 KB
        static_assert(StartCapacity > 0);

        LinearBuffer()
            : capacity_(StartCapacity),
            data_(new T[StartCapacity]),
            head_(0),
            tail_(0),
            base_abs_(0) {}

        ~LinearBuffer() { delete[] data_; }

        LinearBuffer(const LinearBuffer&) = delete;
        LinearBuffer& operator=(const LinearBuffer&) = delete;
        LinearBuffer(LinearBuffer&&) = delete;
        LinearBuffer& operator=(LinearBuffer&&) = delete;

        // ---- Basic geometry ----
        std::size_t size() const noexcept { return tail_ - head_; }
        bool empty() const noexcept { return tail_ == head_; }

        std::uint64_t base_abs() const noexcept { return base_abs_; }
        std::size_t head_local() const noexcept { return head_; }
        std::size_t tail_local() const noexcept { return tail_; }
        std::size_t capacity() const noexcept { return capacity_; }

        // ---- Views ----
        std::span<const T> readable() const noexcept {
            return { data_ + head_, tail_ - head_ };
        }

        std::span<T> readable() noexcept {
            return { data_ + head_, tail_ - head_ };
        }

        // ---- Absolute <-> local (local = index from data_[0]) ----
        std::uint64_t local_to_abs(std::size_t local) const noexcept {
            return base_abs_ + static_cast<std::uint64_t>(local);
        }

        std::size_t abs_to_local(std::uint64_t abs) const noexcept {
            assert(abs >= base_abs_);
            return static_cast<std::size_t>(abs - base_abs_);
        }

        // ---- Readable-relative convenience ----
        // readable-relative 0 == readable().data()[0] == data_[head_]
        std::uint64_t readable_rel_to_abs(std::size_t rel) const noexcept {
            return local_to_abs(head_ + rel);
        }

        std::size_t abs_to_readable_rel(std::uint64_t abs) const noexcept {
            std::size_t local = abs_to_local(abs);
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
        void discard_prefix(std::size_t count) noexcept {
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
            const std::size_t live = tail_ - head_;
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

    private:
        void ensure_writable(std::size_t n) {
            // Fast path
            if (tail_ + n <= capacity_) return;

            // Try compacting first if it would make space
            const std::size_t live = tail_ - head_;
            if (head_ > 0 && live + n <= capacity_) {
                compact();
                return;
            }

            grow(live + n);
        }

        void grow(std::size_t needed_total_live_plus_new) {
            // needed_total_live_plus_new == (tail_-head_) + n
            const std::size_t live = tail_ - head_;
            const std::size_t min_needed_capacity = needed_total_live_plus_new;

            std::size_t new_capacity = capacity_;
            if (new_capacity < min_needed_capacity) {
                // Grow by at least MinGrow, but ensure we hit the needed capacity.
                const std::size_t deficit = min_needed_capacity - new_capacity;
                new_capacity += std::max(deficit, MinGrow);
            } else {
                // Still no room at tail because head_==0 and tail_+n>capacity_:
                // grow by MinGrow.
                new_capacity += MinGrow;
            }

            T* new_data = new T[new_capacity];
            if (live > 0) {
                std::memcpy(new_data, data_ + head_, live * sizeof(T));
            }

            delete[] data_;
            data_ = new_data;
            capacity_ = new_capacity;

            // We copied live bytes down to index 0, so advance base by head_.
            base_abs_ += static_cast<std::uint64_t>(head_);
            head_ = 0;
            tail_ = live;
        }

    private:
        std::size_t capacity_;
        T* data_;
        std::size_t head_;
        std::size_t tail_;
        std::uint64_t base_abs_;
    };

} // namespace Fix
