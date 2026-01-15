#include <fix/core/Framer.hpp>


namespace Fix {
    
    

    void Framer::append(std::string_view data) {
        buffer_.push(std::span<const unsigned char>(reinterpret_cast<const unsigned char*>(data.data()), data.size()));
        auto res = parse_buffer();
        
    }

    bool Framer::parse_buffer() {
        std::string_view data_span = std::string_view(reinterpret_cast<const char*>(buffer_.data().data()), buffer_.data().size());

        if (current_context_.state == FramerContextState::FindingBegin) {
            auto begin_pos = data_span.find(Fix_First_Field);
            if (begin_pos != std::string_view::npos) {
                current_context_.begin = buffer_.readable_rel_to_abs(begin_pos);
            } else {
                return false;
            }
            
            auto body_start = data_span.find("9=", begin_pos);
            if (body_start == std::string_view::npos) {
                return false;
            }
            body_start += 2; // Move past "9="
            auto body_end_pos = data_span.find('\x01', body_start);
            if (body_end_pos == std::string_view::npos) {
                return false;
            }
            std::string_view body_length_str = data_span.substr(body_start, body_end_pos - body_start);
            auto [ptr, ec] = std::from_chars(body_length_str.data(), body_length_str.data() + body_length_str.size(), current_context_.body_len);
            if (ec != std::errc() || ptr != body_length_str.data() + body_length_str.size()) {
                return false; // Might return a small error context here;
            }
            current_context_.state = FramerContextState::ReadingBody;
            auto end = body_end_pos + 1 + current_context_.body_len + 7; // 7 for "10=xxx\x01"
            current_context_.end = buffer_.readable_rel_to_abs(end);
            if (end > data_span.size()) {
                return false;
            }  else {
                // We have a complete message
                current_context_.state = FramerContextState::Complete;
                cached_context_ = current_context_;
                current_context_ = {};
            }
            
        } else {
            //ReadingBody
            auto end = current_context_.end;
            if (end > buffer_.readable_rel_to_abs(buffer_.data().size())) { 
                return false;
            } else {
                current_context_.state = FramerContextState::Complete;
                cached_context_ = current_context_;
                current_context_ = {};
            }
        }
        return cached_context_.state == FramerContextState::Complete;
    }

    bool Framer::has_message() const noexcept {
        return cached_context_.state == FramerContextState::Complete;
    }

    void Framer::consume_message() noexcept {
        if (cached_context_.state != FramerContextState::Complete) return;

        // how many readable bytes until the end of the message?
        std::size_t rel_end = buffer_.abs_to_readable_rel(static_cast<std::size_t>(cached_context_.end));
        buffer_.consume(rel_end);              // drop junk + message bytes up to end
        cached_context_ = {};
    }


    std::string_view Framer::get_message() const noexcept {
        assert(cached_context_.state == FramerContextState::Complete);
        std::size_t rel_begin = buffer_.abs_to_readable_rel(static_cast<std::size_t>(cached_context_.begin));
        std::size_t rel_end = buffer_.abs_to_readable_rel(static_cast<std::size_t>(cached_context_.end));
        auto readable = buffer_.data();
        assert(rel_begin <= readable.size());
        assert(rel_end   <= readable.size());
        assert(rel_end >= rel_begin);

        return std::string_view(
            reinterpret_cast<const char*>(readable.data() + rel_begin),
            rel_end - rel_begin
        );

    }
    

}