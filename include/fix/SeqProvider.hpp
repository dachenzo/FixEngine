#pragma once

namespace Fix {
    struct SeqProvider {
        int next_out();

        int last_in();

        private:
        int seq_ = 1;
    };

    
}