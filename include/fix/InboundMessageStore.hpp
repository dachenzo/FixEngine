#pragma once
#include <memory>
#include <vector>
#include <fix/Message.hpp>


namespace Fix {
    struct InboundMessageStoreSlice {
        bool is_present;
        int seq_num;
        std::unique_ptr<Fix::Message> msg;
    }; 
    struct InboundStore {

        InboundStore() {
            inbound_.reserve(100'000);
        }

        void add(int seq, std::unique_ptr<Fix::Message> msg) {
            
            if (seq > expected_seq_) {
                for (int s = expected_seq_; s < seq; ++s)
                    inbound_.emplace_back(false, s, nullptr);
                inbound_.emplace_back(true, seq, std::move(msg));
                expected_seq_ = seq + 1;  //
            } else if (seq == expected_seq_) {
                inbound_.emplace_back(true, seq, std::move(msg));
                expected_seq_ = seq + 1; 
            } else { // seq < expected_seq_ (dup or late fill in earlier gap)
                const int idx = seq - start_seq_;
                if (idx < 0 || idx >= static_cast<int>(inbound_.size())) return; // outside window
                if (!inbound_[idx].is_present) {
                    inbound_[idx].is_present = true;
                    inbound_[idx].msg = std::move(msg);
                }
            }

        }

        int get_expected_inbound_seq() const {
            return expected_seq_;
        }

        void set_expected_seq(int n) {
            expected_seq_ = n;
        }

        void reset(int start_seq) {
            inbound_.clear();
            start_seq_ = start_seq;
            expected_seq_ = start_seq; 
        }

        private:
        std::vector<InboundMessageStoreSlice> inbound_;
        int start_seq_ = 1;
        int expected_seq_ = 1;
    };

}