#include <fix/core/Framer.hpp>


namespace Fix {
    
    

    void Framer::append(std::string_view data) {
        buffer_.push(std::span<const unsigned char>(reinterpret_cast<const unsigned char*>(data.data()), data.size()));
        while (!completed_messages_.full() && parse_buffer());
        
    }

    //returns true if a new complete message is parsed and available
    bool Framer::parse_buffer() {
        std::string_view readable_view = std::string_view(
            reinterpret_cast<const char*>(buffer_.readable().data()),
            buffer_.readable().size()
        );
        auto readable_begin_abs = buffer_.readable_rel_to_abs(0);
        if (scan_abs_ < readable_begin_abs) scan_abs_ = readable_begin_abs;


        auto relative_start = buffer_.abs_to_readable_rel(scan_abs_);
        if (relative_start >= readable_view.size()) {
            // Nothing new to scan
            return false;
        }

        if (current_context_.state == FramerContextState::FindingBegin) {
            auto begin_pos = readable_view.find(Fix_First_Field, relative_start);
            if (begin_pos != std::string_view::npos) {
                current_context_.begin = buffer_.readable_rel_to_abs(begin_pos);
                scan_abs_ = current_context_.begin;
            } else if (completed_messages_.empty()) {
                // No messages found at all: can discard all the readable data
                constexpr std::uint64_t k = Fix_First_Field.size();
                std::uint64_t keep = (readable_view.size() < k-1) ? readable_view.size() : (k-1);
                std::uint64_t drop = readable_view.size() - keep;
                buffer_.discard_prefix(drop);      // cheap
                scan_abs_ = buffer_.readable_rel_to_abs(0); // reset scan to start of remaining suffix
                return false;
            } else {
                return false;
            }
            
            auto body_start = readable_view.find("9=", begin_pos);
            if (body_start == std::string_view::npos ) {
                return false;
            } else if (body_start == std::string_view::npos && readable_view.size() - begin_pos > MAX_BEGIN_TO_BODYLEN_FIELD_BYTES) {
                // Too far from begin to be valid
                scan_abs_ = current_context_.begin + 1;  // begin is ABS
                current_context_ = {};
                return false;
            }

            body_start += 2; // Move past "9="
            auto body_end_pos = readable_view.find('\x01', body_start);
            if (body_end_pos == std::string_view::npos) {
                return false;
            }
            std::string_view body_length_str = readable_view.substr(body_start, body_end_pos - body_start);
            auto [ptr, ec] = std::from_chars(body_length_str.data(), body_length_str.data() + body_length_str.size(), current_context_.body_len);
            if (ec != std::errc() || ptr != body_length_str.data() + body_length_str.size()) {
                scan_abs_ = current_context_.begin + 1;  // begin is ABS
                current_context_ = {};
                return false; // Might return a small error context here;
            }
            if (current_context_.body_len > MAX_MESSAGE_SIZE) {
                // Don't trust computed end. Just move scan forward slightly and try again.
                scan_abs_ = current_context_.begin + 1;  // begin is ABS
                current_context_ = {};
                return false;
            }
            current_context_.state = FramerContextState::ReadingBody;
            auto end = body_end_pos + 1 + current_context_.body_len + 7; // 7 for "10=xxx\x01"
            current_context_.end = buffer_.readable_rel_to_abs(end);
            if (end > readable_view.size()) {
                return false;
            }  else {
                // We have a complete message
                current_context_.state = FramerContextState::Complete;
                scan_abs_ = current_context_.end;
                completed_messages_.push(make_message_window(current_context_));
                current_context_ = {};
                return true;
            }
            
        } else {
            //ReadingBody
            auto end = current_context_.end;
            if (end > buffer_.readable_rel_to_abs(readable_view.size())) { 
                return false;
            } else {
                current_context_.state = FramerContextState::Complete;
                scan_abs_ = current_context_.end;
                completed_messages_.push(make_message_window(current_context_));
                current_context_ = {};
                return true;
            }
        }
        return false;
    }

    bool inline Framer::has_message() const noexcept {
        return completed_messages_.size() > 0;
    }

    void Framer::consume_message() {
        assert(has_message());
        auto earliest_window = completed_messages_.front();
        buffer_.discard_prefix(buffer_.abs_to_readable_rel(static_cast<std::uint64_t>(earliest_window.end)));
        completed_messages_.pop_front();
    }


    std::string_view Framer::get_message() const noexcept {
        assert(has_message());
        auto earliest_window = completed_messages_.front();
        std::uint64_t rel_begin = buffer_.abs_to_readable_rel(static_cast<std::uint64_t>(earliest_window.begin));
        std::uint64_t rel_end = buffer_.abs_to_readable_rel(static_cast<std::uint64_t>(earliest_window.end));
        auto readable = buffer_.readable();
        assert(rel_begin <= readable.size());
        assert(rel_end   <= readable.size());
        assert(rel_end >= rel_begin);

        return std::string_view(
            reinterpret_cast<const char*>(readable.data() + rel_begin),
            rel_end - rel_begin
        );

    }

    void Framer::discard_n_from_head(std::uint64_t count) { 
        assert(count <= buffer_.readable_size());
        buffer_.discard_prefix(count);
    }

    
    

}