#pragma once
#include <concepts>
#include <cstdint>
#include <array>
#include <atomic>

namespace Fix::Log {

    template<std::movable Data>
    struct RingCell {
        std::atomic<uint64_t> epoch;
        Data data;
    };


    template <std::movable Data>
    struct MpscRing {

        static constexpr const  uint64_t capacity = 1 << 13; //always has to be a power of 2 for & modulo to work  
        static constexpr const uint64_t mask = capacity - 1;

    

        MpscRing(const MpscRing& other) = delete;
        MpscRing& operator=(const MpscRing& other) = delete;
        MpscRing(const MpscRing&& other) = delete;
        MpscRing& operator=(const MpscRing&& other) = delete;

        bool try_push(Data& data) {
            auto ticket = head_.fetch_add(1, std::memory_order_acq_rel);
            RingCell& c = buff_[ticket & mask];
            auto curr_epoch = c.epoch.load(std::memory_order_acquire);

            if (ticket != curr_epoch) return false;

            c.data = std::move(data);
            c.epoch.store(ticket+1, std::memory_order_release);
            return true;
            
                
        }

        void push(Data& data) {
            auto ticket = head_.fetch_add(1, std::memory_order_acq_rel);
            RingCell& c = buff_[ticket & mask];

            while (ticket != c.epoch.load(std::memory_order_acquire)) {

            }
            c.data = std::move(data);
            c.epoch.store(ticket+1, std::memory_order_release);

        }


        bool pop(Data& out) {
            RingCell& cell = buff_[tail_&mask];
            uint64_t expect = tail+1;
            if (cell.epoch.load(std::memory_order_acquire) != expect) return false;
            out = std::move(cell.data);
            cell.epoch.store(tail_+capacity, std::memory_order_release);
            tail_++;
            return true;
        }


        private:
        std::array<RingCell<Data>, capacity> buff_;
        std::atomic<uint64_t> head_;
        uint64_t tail_;


    };
}
