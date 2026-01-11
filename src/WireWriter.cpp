#include <fix/core/WireWriter.hpp>


namespace Fix {

    WireWriter WireWriter::from_arena(Fix::Arena& arena, std::string_view msg_wire) {
        auto handle = arena.allocate(msg_wire.size());
        WireWriter writer(std::move(handle));
        writer.append(msg_wire);
        return writer;
    }

    WireWriter::WireWriter(ArenaHandle&& handle) 
        :handle_(std::move(handle)), size_(0) {
    }

    void WireWriter::append(std::string_view data) {
        append(data.data(), data.size());
    }

    void WireWriter::append(const char* data, std::size_t size) {
        // if u segfault here, you didnt make sure the ArenaHandle has enough capacity
        std::memcpy(handle_.data() + size_, data, size);
        size_ += size;
    }

    void WireWriter::append(char c) {
        append(&c, 1);
    }

    void WireWriter::append_eq() {
        append('=');
    }

    void WireWriter::append_int(int64_t value) {
        auto r = std::to_chars(int_buff_, int_buff_ + sizeof(int_buff_), value);
        append(int_buff_, static_cast<std::size_t>(r.ptr - int_buff_));
    }   

    void WireWriter::append_soh() {
        append('\x01');
    }

    std::byte* WireWriter::data() const noexcept {
        return handle_.data();
    }
    std::size_t WireWriter::size() const noexcept {
        return size_;
    }

    

    std::string_view WireWriter::view() const noexcept {
        return std::string_view(reinterpret_cast<const char*>(handle_.data()), size_);
    }


}