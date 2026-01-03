#include <fix/core/WireWriter.hpp>


namespace Fix {

    WireWriter::WireWriter(ArenaHandle& handle) 
        :handle_(handle), size_(0) {
    }

    void WireWriter::append(std::string_view data) {
        append(data.data(), data.size());
    }

    void WireWriter::append(const char* data, std::size_t size) {
        if (size_ + size > handle_.capacity()) {
            // should not happen if used correctly
            throw std::runtime_error("WireWriter: Exceeding allocated capacity");
        }
        std::memcpy(handle_.data() + size_, data, size);
        size_ += size;
    }

    void WireWriter::append(char c) {
        append(&c, 1);
    }

    void WireWriter::append_eq() {
        append('=');
    }

    void WireWriter::append_soh() {
        append('\x01');
    }

    

    std::string_view WireWriter::view() const noexcept {
        return std::string_view(reinterpret_cast<const char*>(handle_.data()), size_);
    }


}