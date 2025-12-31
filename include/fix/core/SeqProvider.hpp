#pragma once
#include <fix/core/MessageStore.hpp>

namespace Fix {
    class SeqProvider {
        std::size_t incoming_seq_number_{1};
        std::size_t outgoing_seq_number_{1};

        public:

        std::size_t next_out() const noexcept {
            return outgoing_seq_number_;
        }

        std::size_t next_in() const noexcept {
            return incoming_seq_number_;
        }
    };

    
}