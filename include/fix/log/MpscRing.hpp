#pragma once
#include <cstdint>




namespace Fix::Log {

    #if defined(__x86_64__) || defined(_M_X64)
        #include <immintrin.h>
        inline void cpu_relax() { _mm_pause(); }
    #elif defined(__arm__) || defined(_M_ARM) || defined(__aarch64__) || defined(_M_ARM64)
        #include <arm_acle.h>
        inline void cpu_relax() { __yield(); }
    #else
        #include <thread>
        inline void cpu_relax() { std::this_thread::yield(); }
    #endif

    template<std::movable Data>
    struct RingCell {
        std::atomic<uint64_t> epoch;
        Data data;
    };


    template <std::movable Data>
    struct MpscRing {

        static constexpr const  uint64_t capacity = 1 << 10; //always has to be a power of 2 for & modulo to work  
        static_assert(capacity%2 == 0, "MpscRing capacity must be a power of 2");
        static constexpr const uint64_t mask = capacity - 1;

        MpscRing() {
            for (uint64_t i = 0; i < capacity; ++i) buff_[i].epoch.store(i, std::memory_order_relaxed);
        } 

        MpscRing(MpscRing& other) = delete;
        MpscRing& operator=(MpscRing& other) = delete;
        MpscRing(MpscRing&& other) = delete;
        MpscRing& operator=(MpscRing&& other) = delete;

        bool try_push(Data&& data) {
            auto ticket = head_.fetch_add(1, std::memory_order_acq_rel);
            RingCell<Data>& c = buff_[ticket & mask];
            auto curr_epoch = c.epoch.load(std::memory_order_acquire);

            if (ticket != curr_epoch) return false;

            c.data = std::move(data);
            c.epoch.store(ticket+1, std::memory_order_release);
            return true;
            
                
        }

        void push(Data&& data) {
            auto ticket = head_.fetch_add(1, std::memory_order_acq_rel);
            RingCell<Data>& c = buff_[ticket & mask];

            while (ticket != c.epoch.load(std::memory_order_acquire)) {
                cpu_relax();
            }
            c.data = std::move(data);
            c.epoch.store(ticket+1, std::memory_order_release);

        }


        bool pop(Data& out) {
            RingCell<Data>& cell = buff_[tail_&mask];
            uint64_t expect = tail_+1;
            if (cell.epoch.load(std::memory_order_acquire) != expect) return false;
            out = std::move(cell.data);
            cell.epoch.store(tail_+capacity, std::memory_order_release);
            tail_++;
            return true;
        }


        private:
        std::array<RingCell<Data>, capacity> buff_;
        std::atomic<uint64_t> head_{0};
        uint64_t tail_{0};


    };
}
