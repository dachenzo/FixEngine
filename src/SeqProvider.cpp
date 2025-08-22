#include <fix/SeqProvider.hpp>


namespace Fix {
    int SeqProvider::next_out() {
        return seq_++;
    }
}