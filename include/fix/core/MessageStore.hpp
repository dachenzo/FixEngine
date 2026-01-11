#pragma once
#include <span>
#include <vector>
#include <string>
#include <cstring>
#include <unordered_set>
#include <fix/core/InboundMessageStore.hpp>
#include <fix/core/OutboundMessageStore.hpp>
#include <fix/core/Message.hpp>


namespace Fix {

        

    struct MsgIndex {
        uint32_t off;      // offset into blob buffer
        uint32_t len;      // byte length
        uint32_t seq;      // MsgSeqNum
        

        // patch points (offsets relative to off)
        int32_t off_34 = -1;  
        int32_t off_52 = -1;  
        int32_t off_43 = -1;  
        int32_t off_122 = -1;
        uint16_t len_34 = 0; 
        uint16_t len_43 = 0;  
        uint16_t len_122 = 0;   
        uint16_t len_52 = 0;
        std::array<char, 2> msg_type; 
        u_int8_t msg_size = 1;

        

        std::string_view get_msg_type() const noexcept {
            return std::string_view(msg_type.data(), msg_size);
        }
    };



    struct ResendAction {
        std::size_t begin_seq_no;
        std::size_t end_seq_no;
        bool gap_fill;  
    };

   

    struct ResendStream {

        ResendStream(std::byte* blob_buffer,
                     std::uint32_t blob_buffer_size,
                     std::uint32_t begin_seq_no,
                     std::uint32_t end_seq_no,
                     std::span<const MsgIndex> outbound_index
        )  :  outbound_index_(outbound_index),
              blob_buffer_(blob_buffer),
              blob_buffer_size_(blob_buffer_size),
              begin_seq_no_(begin_seq_no),
              current_index_(begin_seq_no),
              end_seq_no_(end_seq_no)
        {
    
        }

        static bool no_resend(std::string_view msg_type) {
            if (msg_type.size() == 1) {
                switch(msg_type[0]) {
                    case '0': // Heartbeat
                    case 'A': // Logon
                    case '1': // Test Request
                    case '3': // Reject
                    case '4': // Sequence Reset
                    case '5': // Logout
                        return true;
                    default:
                        return false;
                }
            }
            return false;
        }


        bool has_next() const noexcept {
            return current_index_ <= end_seq_no_;
        }

        

        ResendAction next() {
            // ResendAction range is inclusive: [begin_seq_no, end_seq_no]
            std::size_t start = current_index_;
            bool gap_fill = false;
            

            while (current_index_ <= end_seq_no_ && no_resend(outbound_index_[current_index_ - 1].get_msg_type())) {
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
        std::span<const MsgIndex> outbound_index_;
        std::byte* blob_buffer_;
        std::uint32_t blob_buffer_size_;
        std::uint32_t current_index_ = 0;
        std::uint32_t begin_seq_no_;
        std::uint32_t end_seq_no_;

    };

   
    struct MessageStore {
        static constexpr std::size_t blob_buffer_start_size = 8 * 1024 * 1024; // 8 MB
        static constexpr std::size_t min_blob_buffer_grow_size = 4 * 1024 * 1024; // 4 MB
        static constexpr std::size_t message_index_reserve_size = blob_buffer_start_size / 2048; // avg 2KB per message


        

        MessageStore();
        ~MessageStore();
        MessageStore(const MessageStore&) = delete;
        MessageStore& operator=(const MessageStore&) = delete;
        MessageStore(MessageStore&&) = delete;
        MessageStore& operator=(MessageStore&&) = delete;

        void store_outbound_message(std::string_view wire);

        static MsgIndex create_message_index(std::string_view wire, uint32_t off); 

        ResendStream get_resend_stream(std::uint32_t begin_seq_no, std::uint32_t end_seq_no) const;

        std::uint32_t capacity() const noexcept;

        std::uint32_t size() const noexcept;
        
        std::string_view get_message_wire(const MsgIndex& index) const noexcept;

        std::string_view all_messages() const noexcept;

        const MsgIndex& get_message_index(std::size_t seq_num) const noexcept; 


        

        private:
        void grow_blob_buffer_(std::size_t min_additional_size);
        void grow_blob_buffer_();
    
        std::vector<MsgIndex> outbound_index_;
        std::byte* blob_buffer_ = nullptr;
        std::uint32_t blob_buffer_size_ = 0;
        std::uint32_t blob_buffer_used_ = 0;
        
    };
}