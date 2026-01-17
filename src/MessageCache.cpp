#include <fix/core/RecoveryCache.hpp>


namespace Fix {

    RecoveryCache::RecoveryCache()
        : data_(nullptr),
          buff_capacity_(0),
          buff_size_(0),
          base_offset_(0),
          size_(0) {
        grow_buffer(initial_capacity);
        std::fill(slots_.begin(), slots_.end(), KEmpty);
    }

    RecoveryCache::~RecoveryCache() {
        delete[] data_;
    }

    bool inline RecoveryCache::in_window(SeqNum seqnum) const noexcept {
        if (seqnum < base_offset_) return false;
        return static_cast<std::uint64_t>(seqnum - base_offset_) < window;
    }   


    std::uint16_t inline RecoveryCache::slot_index(SeqNum seqnum) const noexcept {
        assert(in_window(seqnum) && "SeqNum out of window");
        return static_cast<std::uint16_t>(seqnum - base_offset_);
    }

    bool inline RecoveryCache::empty() const noexcept {
        return size_ == 0;
    }

    void RecoveryCache::start(SeqNum seqnum) {
        reset();
        base_offset_ = seqnum;
    }

    bool inline RecoveryCache::contains(SeqNum seqnum) const noexcept {
        return in_window(seqnum) && slots_[slot_index(seqnum)] != KEmpty;
    }

    void RecoveryCache::insert(SeqNum seqnum, std::string_view msg_wire) {
        // Insert message into cache
        // Assumes message for seqnum does not already exist
        assert(in_window(seqnum) && "SeqNum out of window");
        assert(!contains(seqnum) && "Message for SeqNum already exists in RecoveryCache");
        MessageBounds bounds = insert_message(msg_wire);
        slots_[slot_index(seqnum)] = bounds;
        ++size_;
    }

    std::string_view RecoveryCache::get(SeqNum seqnum) {
        assert(contains(seqnum) && "Message not found in RecoveryCache");
        MessageBounds bounds = slots_[slot_index(seqnum)];
        return std::string_view{reinterpret_cast<const char*>(data_ + bounds.offset), bounds.size};
    }

    void RecoveryCache::consume(SeqNum seqnum) {
        assert(contains(seqnum) && "Message not found in RecoveryCache");
        slots_[slot_index(seqnum)] = KEmpty;
        --size_;
        if (size_ == 0) {
            buff_size_ = 0;
        }
    }


    MessageBounds RecoveryCache::insert_message(std::string_view msg_wire) {
        assert(msg_wire.size() > 0 && "Cannot insert empty message");
        if (buff_size_ + msg_wire.size() > buff_capacity_) {
            // need to grow
            std::uint64_t extra_needed = (buff_size_ + msg_wire.size()) - buff_capacity_;
            std::uint64_t grow_by = std::max(extra_needed, min_capacity_growth);
            grow_buffer(grow_by); 
        }

        std::uint64_t offset = buff_size_;
        std::memcpy(data_ + buff_size_, msg_wire.data(), msg_wire.size());
        buff_size_ += msg_wire.size();

        return MessageBounds{offset, static_cast<std::uint64_t>(msg_wire.size())};
    }

    void RecoveryCache::grow_buffer(std::uint64_t extra_capacity) {
        std::uint64_t new_capacity = buff_capacity_ + extra_capacity;
        auto new_data = new std::byte[new_capacity];
        if (data_) {
            std::memcpy(new_data, data_, buff_size_);
            delete[] data_;
        }
        data_ = new_data;
        buff_capacity_ = new_capacity;
    
    }

    void RecoveryCache::reset() {
        assert(empty() && "Cannot reset RecoveryCache with existing messages");
        buff_size_ = 0;
        base_offset_ = 0;
        size_ = 0;
        std::fill(slots_.begin(), slots_.end(), KEmpty);
    }

}

