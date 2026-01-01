#pragma once
#include <span>
#include <vector>
#include <string>
#include <unordered_set>
#include <fix/core/InboundMessageStore.hpp>
#include <fix/core/OutboundMessageStore.hpp>
#include <fix/core/Message.hpp>


namespace Fix {

    struct OutboundMessage {
        ValidMessage message_;
        std::string wire_;
    };

    struct ResendAction {
        std::size_t begin_seq_no;
        std::size_t end_seq_no;
        bool gap_fill;  
    };

   

    struct ResendStream {

        ResendStream(std::span<const OutboundMessage> messages,
                     std::size_t begin_seq_no,
                     std::size_t end_seq_no)
            : messages_(messages),
              begin_seq_no_(begin_seq_no),
              current_index_(begin_seq_no),
              end_seq_no_(end_seq_no)
        {
    
        }


       ResendAction next() {
            if (current_index_ > end_seq_no_) {
                return end;
            }

            std::size_t start = current_index_;
            bool gap_fill = false;
            while (skipped_seq_nos_.count(current_index_) && current_index_ <= end_seq_no_) {
                current_index_++;
                gap_fill = true;   
            }

            std::size_t end = gap_fill ? current_index_ - 1 : current_index_;
            if (!gap_fill) {
                current_index_++;
            }

            return ResendAction{start, end, gap_fill};
       }



        private:
        static const ResendAction end;
        std::unordered_set<std::size_t> skipped_seq_nos_;
        std::span<const OutboundMessage> messages_;
        std::size_t current_index_ = 0;
        std::size_t begin_seq_no_;
        std::size_t end_seq_no_;

    };

   
    struct MessageStore {

        void store_outbound_message(const ValidMessage& message, std::string& wire);

        std::span<const OutboundMessage> get_resend_wires(std::size_t begin_seq_no, std::size_t end_seq_no);



        private:

        void create_new_wire_();


        std::vector<ValidMessage> inbound_buffer_;
        std::vector<OutboundMessage> outbound_buffer_;
    };
}