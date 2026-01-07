#include <fix/core/MessageFactory.hpp>


namespace Fix {

    FactoryScratch::FactoryScratch() {
        buffer = new std::byte[buffer_size];
    }

    FactoryScratch::~FactoryScratch() {
        delete[] buffer;
    }

    void FactoryScratch::reset() {
        body_length_offset = 0;
        position = 0;
        body_length_digit_size = 32;
        checksum_offset = 0;
    }

    std::size_t FactoryScratch::compute_body_length() const noexcept {
        return position - (body_length_offset + body_length_digit_size + 1); // +1 for SOH
    }

    std::size_t FactoryScratch::compute_checksum() const noexcept {
        uint64_t sum = 0;
        for (std::size_t i = 0; i < position; ++i) {
            sum += static_cast<unsigned char>(buffer[i]);
        }
        return sum % 256;
    }

    void FactoryScratch::insert_checksum() {
        auto checksum = compute_checksum();
        auto r = std::to_chars(int_buff, int_buff + sizeof(int_buff), checksum);
        std::size_t len = static_cast<std::size_t>(r.ptr - int_buff);
        add_char(10);
        add_equal_sign();
        if (len < 3) {
            // zero-pad
            for (std::size_t i = 0; i < 3 - len; ++i) {
                add_char(static_cast<unsigned char>('0'));
            }
        }
        add_string(std::string_view{int_buff, len});
        add_soh();
    }

    void FactoryScratch::insert_body_length() {
        auto r = std::to_chars(int_buff, int_buff + sizeof(int_buff), compute_body_length());
        std::size_t len = static_cast<std::size_t>(r.ptr - int_buff);

        std::memcpy(
            buffer + body_length_offset,
            int_buff,
            len
        );
        std::memmove(
            buffer + body_length_offset + len,
            buffer + body_length_offset + body_length_digit_size,
            position - (body_length_offset + body_length_digit_size)
        ); // shift left;

        position -= (body_length_digit_size - len);
        body_length_digit_size = len;
    }



    void FactoryScratch::add_int(int64_t value) {
        auto r = std::to_chars(int_buff, int_buff + sizeof(int_buff), value);
        add_string(std::string_view{int_buff, static_cast<std::size_t>(r.ptr - int_buff)});
    }

    void FactoryScratch::add_string(std::string_view sv) {
        if (position + sv.size() > buffer_size) {
            auto extra = position + sv.size() - buffer_size;
            grow(std::min(extra, min_grow_size));
        }
        std::memcpy(buffer + position, sv.data(), sv.size());
        position += sv.size();
    }

    void FactoryScratch::add_body_length_placeholder() {
        add_int(9);
        add_equal_sign();
        body_length_offset = position;
        for (std::size_t i = 0; i < body_length_digit_size; ++i) {
            add_char(static_cast<unsigned char>('0'));
        }
        add_soh();
    }

    void FactoryScratch::add_equal_sign() { 
        add_char(static_cast<unsigned char>('='));
    }

    std::size_t FactoryScratch::get_position() const {
        return position;
    }

    std::size_t FactoryScratch::get_buffer_size() const {
        return buffer_size;
    }

    std::string_view FactoryScratch::get_buffer_view() {
        return std::string_view{
            reinterpret_cast<const char*>(buffer),
            position
        };
    }

    void FactoryScratch::add_soh() {
        add_char(static_cast<unsigned char>(0x01));
    }

    void FactoryScratch::add_char(unsigned char c) {
        if (position + 1 > buffer_size) {
            grow(min_grow_size);
        }
        buffer[position] = std::byte{c};
        position += 1;
    }

    void FactoryScratch::add_field(int64_t tag, std::string_view value) {
        add_int(tag);
        add_equal_sign();
        add_string(value);
        add_soh();
    }

    void FactoryScratch::grow(std::size_t extra) {
        std::size_t new_size = buffer_size + extra;
        std::byte* new_buffer = new std::byte[new_size];
        std::memcpy(new_buffer, buffer, position);
        delete[] buffer;
        buffer = new_buffer;
        buffer_size = new_size;
    }

}