#include <fix/core/SeqProvider.hpp>


namespace Fix {

    SeqProvider::SeqProvider(Fix::MessageStore& store): store_{store} {}
    
    int SeqProvider::next_out() const noexcept {
        return store_.get_next_sender_seq();
    }

    int SeqProvider::last_in() const noexcept {
        return store_.get_last_in_outbound();
    }

}