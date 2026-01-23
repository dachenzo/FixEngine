#include <algorithm>
#include <span>
#include <charconv>
#include <fix/core/MessageStore.hpp>



namespace Fix {
    
    MessageStore::MessageStore() {
        blob_buffer_size_ = blob_buffer_start_size;
        blob_buffer_ = new std::byte[blob_buffer_size_];
        outbound_index_.reserve(message_index_reserve_size);
    }

    MessageStore::~MessageStore() {
        delete[] blob_buffer_;
    }

    MsgIndex MessageStore::create_message_index(std::string_view wire, uint32_t off) {
        // Called on a wire representing a complete FIX message
        MsgIndex idx{};
        idx.off = off;
        idx.len = static_cast<uint32_t>(wire.size());

        // parse for patch points
        std::size_t pos = 0;
        while (pos < wire.size()) {
            auto eq_pos = wire.find('=', pos);
            assert(eq_pos != std::string_view::npos); // shouldnt happen
            auto soh_pos = wire.find('\x01', eq_pos+1);
            assert(soh_pos != std::string_view::npos); // shouldnt happen
            auto tag = std::string_view{wire.data()+pos, eq_pos-pos};
            auto value = std::string_view{wire.data()+eq_pos+1, soh_pos - eq_pos -1};

            if (tag == "34") {
                idx.off_34 = static_cast<int32_t>(eq_pos + 1);
                idx.len_34 = static_cast<uint16_t>(value.size());
                uint32_t seq{};
                auto [p, ec] = std::from_chars(value.data(), value.data() + value.size(), seq);
                assert(ec == std::errc{}); // shouldnt happen
                idx.seq = seq;

            } else if (tag == "52") {
                idx.off_52 = static_cast<int32_t>(eq_pos + 1);
                idx.len_52 = static_cast<uint16_t>(value.size());
            } else if (tag == "43") {
                idx.off_43 = static_cast<int32_t>(eq_pos + 1);
                idx.len_43 = static_cast<uint16_t>(value.size());
            } else if (tag == "122") {
                idx.off_122 = static_cast<int32_t>(eq_pos + 1);
                idx.len_122 = static_cast<uint16_t>(value.size());
            } else if (tag == "35") {
                idx.msg_size = static_cast<uint8_t>(std::min(value.size(), static_cast<std::size_t>(2)));
                std::copy_n(value.data(), idx.msg_size, idx.msg_type.data());
            } else if (tag == "10") {
                // done
                break;
            }
            pos = soh_pos + 1;
        }    
        return idx;
    }

    void MessageStore::grow_blob_buffer_(std::size_t additional_size) {
        std::size_t new_size = blob_buffer_size_ + std::max(min_blob_buffer_grow_size, additional_size);
        std::byte* new_buffer = new std::byte[new_size];
        std::memcpy(new_buffer, blob_buffer_, blob_buffer_used_);
        delete[] blob_buffer_;
        blob_buffer_ = new_buffer;
        blob_buffer_size_ = new_size;
    }


    void MessageStore::grow_blob_buffer_() {
        grow_blob_buffer_(min_blob_buffer_grow_size);
    }

    void MessageStore::store_outbound_message(std::string_view wire) {
        if (blob_buffer_used_ + wire.size() > blob_buffer_size_) {
            grow_blob_buffer_(wire.size()+blob_buffer_used_ - blob_buffer_size_);
        }
        std::memcpy(blob_buffer_ + blob_buffer_used_, wire.data(), wire.size());
        auto msg_index = create_message_index(wire, static_cast<uint32_t>(blob_buffer_used_));
        assert(msg_index.seq == outbound_index_.size() + 1); // MsgSeqNum should be sequential
        outbound_index_.push_back(msg_index);
        blob_buffer_used_ += wire.size();
    }

    ResendStream MessageStore::get_resend_stream(std::uint32_t begin_seq_no, std::uint32_t end_seq_no) const {
        return ResendStream(
            blob_buffer_,
            blob_buffer_used_,
            begin_seq_no,
            end_seq_no,
            outbound_index_
        );
    }

    std::uint32_t MessageStore::capacity() const noexcept {
        return blob_buffer_size_;
    }

    std::uint32_t MessageStore::size() const noexcept {
        return blob_buffer_used_;
    }

    std::string_view MessageStore::all_messages() const noexcept {
        return std::string_view{reinterpret_cast<const char*>(blob_buffer_), blob_buffer_used_};
    }

    const MsgIndex& MessageStore::get_message_index(std::size_t seq_num) const noexcept {
        assert(seq_num > 0 && seq_num <= outbound_index_.size());
        return outbound_index_[seq_num - 1];
    }

    std::string_view MessageStore::get_message_wire(const MsgIndex& index) const noexcept {
        return std::string_view{
            reinterpret_cast<const char*>(blob_buffer_ + index.off),
            index.len
        };
    }
}