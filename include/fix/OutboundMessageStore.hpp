#pragma once
#include <memory>
#include <cassert>
#include <algorithm>
#include <vector>
#include <fix/Message.hpp>

namespace Fix {
    struct OutboundMessaegStoreSlice {
        int seq_num;
        std::unique_ptr<Fix::Message> msg;
    };
#
    struct OutboundStore {
        OutboundStore() {
            outbound_.reserve(100'000);
        }

        void add (int seq_num, std::unique_ptr<Fix::Message> msg) {
            assert(seq_num == outbound_seq_ && "outbound seq mismatch");
            outbound_.emplace_back(seq_num, std::move(msg));
            outbound_seq_++;
        }

        std::span<const OutboundMessaegStoreSlice>
        get_message_range(int begin, int end_inclusive_or_0_for_inf) {
            const int start = std::max(begin, start_seq_) - start_seq_;
            int end_excl = (end_inclusive_or_0_for_inf == 0)
                ? static_cast<int>(outbound_.size())
                : (end_inclusive_or_0_for_inf - start_seq_ + 1); // +1 for exclusive
            end_excl = std::clamp(end_excl, 0, static_cast<int>(outbound_.size()));
            if (start < 0 || start > end_excl) return {};
            return { outbound_.data() + start, static_cast<size_t>(end_excl - start) };
        }

        int get_next_outbound_seq() const { return outbound_seq_;}

        void reset(int new_seq) {
            start_seq_ = new_seq;
            outbound_seq_ = new_seq;
        }

        void set_outbound_seq(int n) {
            outbound_seq_ = n;
        }

        private:
        std::vector<OutboundMessaegStoreSlice> outbound_;
        int start_seq_ = 1;
        int outbound_seq_ = 1;
    };
    

}