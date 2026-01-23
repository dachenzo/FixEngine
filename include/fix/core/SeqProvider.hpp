#pragma once
#include <fix/core/MessageStore.hpp>

namespace Fix {
    class SeqProvider {
        std::size_t incoming_seq_number_{1};
        std::size_t outgoing_seq_number_{1};

        public:

        void update_in(std::size_t seq_no) noexcept {
            incoming_seq_number_ = seq_no;
        };
        void update_out(std::size_t seq_no) noexcept {
            outgoing_seq_number_ = seq_no;
        };
        std::size_t next_out() const noexcept {
            return outgoing_seq_number_;
        }

        std::size_t next_in() const noexcept {
            return incoming_seq_number_;
        }

        void reset() noexcept {
            incoming_seq_number_ = 1;
            outgoing_seq_number_ = 1;
        }

        void acceptor_reset() noexcept {
            incoming_seq_number_ = 2;
            outgoing_seq_number_ = 1;
        }
    };

    
}