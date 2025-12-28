#pragma once
#include <span>
#include <fix/core/InboundMessageStore.hpp>
#include <fix/core/OutboundMessageStore.hpp>
#include <fix/core/Message.hpp>


namespace Fix {

   
    struct MessageStore {
        
        int  get_next_sender_seq() const;
        void set_next_sender_seq(int n);
        int  get_expected_target_seq() const;
        void set_expected_target_seq(int n);

        int get_last_in_outbound() const;

        // outbound persistence
        void store_outbound(int seq, Fix::Message& msg);   // after stamping 8/9/35/... and before send
        std::span<const Fix::OutboundMessaegStoreSlice> get_outbound_range(int begin, int end_inclusive_or_0_for_inf) const;

        // inbound (optional but recommended)
        void store_inbound(int seq, Fix::Message& msg);
 
        // lifecycle
        void reset(int next_sender = 1, int expected_target = 1);

        private:
        Fix::InboundStore inbound_;
        Fix::OutboundStore outbound_;
        

    };
}