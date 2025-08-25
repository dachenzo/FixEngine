#pragma once
#include <fix/MessageStore.hpp>

namespace Fix {
    struct SeqProvider {

        SeqProvider(Fix::MessageStore& store);

        int next_out() const noexcept;

        int last_in() const noexcept;

        private:
        Fix::MessageStore& store_;
    };

    
}