#include <algorithm>
#include <memory>
#include <span>
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
            if (eq_pos == std::string_view::npos) break; // shouldnt happen
            auto soh_pos = wire.find('\x01', pos);
            if (soh_pos == std::string_view::npos) break; // shouldnt happen
            auto tag = std::string_view{wire.data()+pos, eq_pos-pos};
            auto value = std::string_view{wire.data()+eq_pos+1, soh_pos - eq_pos -1};

            if (tag == "34") {
                idx.off_34 = static_cast<int32_t>(eq_pos + 1 - off);
                idx.len_34 = static_cast<uint16_t>(value.size());
                idx.seq = static_cast<uint32_t>(std::stoul(std::string(value)));
            } else if (tag == "52") {
                idx.off_52 = static_cast<int32_t>(eq_pos + 1 - off);
                idx.len_52 = static_cast<uint16_t>(value.size());
            } else if (tag == "43") {
                idx.off_43 = static_cast<int32_t>(eq_pos + 1 - off);
                idx.len_43 = static_cast<uint16_t>(value.size());
            } else if (tag == "122") {
                idx.off_122 = static_cast<int32_t>(eq_pos + 1 - off);
                idx.len_122 = static_cast<uint16_t>(value.size());
            } else if (tag == "35") {
                if (value.size() == 2) {
                    idx.is_lenghth_2 = true;
                }
                std::copy_n(value.data(), std::min(value.size(), static_cast<std::size_t>(2)), idx.msg_type.data());
            } else if (tag == "10") {
                // done
                break;
            }
            pos = soh_pos + 1;
        }    
        return idx;
    }


    
}