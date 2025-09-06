#include <algorithm>
#include <memory>
#include <span>
#include <fix/MessageStore.hpp>



namespace Fix {
    std::span<const Fix::OutboundMessaegStoreSlice>
    MessageStore::get_outbound_range(int begin, int end_inclusive_or_0_for_inf) const {
        return outbound_.get_message_range(begin, end_inclusive_or_0_for_inf);
    }

    void MessageStore::reset(int next_sender, int expected_target) {
        outbound_.reset(next_sender);
        inbound_.reset(expected_target);
    }

    void MessageStore::store_inbound(int seq, Fix::Message& msg) {
        inbound_.add(seq, std::make_unique<Fix::Message>(std::move(msg)));
    }

    void MessageStore::store_outbound(int seq, Fix::Message& msg) {
        outbound_.add(seq, std::make_unique<Fix::Message>(std::move(msg)));
    }

    int MessageStore::get_next_sender_seq() const {
        return outbound_.get_next_outbound_seq();
    }

    int MessageStore::get_expected_target_seq() const {
        return inbound_.get_expected_inbound_seq();
    }

    int MessageStore::get_last_in_outbound() const {
        return inbound_.get_last_in();
    }

    void MessageStore::set_expected_target_seq(int n) {
        inbound_.set_expected_seq(n);
    }

    void MessageStore::set_next_sender_seq(int n) {
        outbound_.set_outbound_seq(n);
    }

}