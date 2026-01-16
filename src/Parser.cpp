#include <string>
#include <cstring>
#include <fix/core/Parser.hpp>
#include <fix/core/Message.hpp>

namespace Fix {

    bool Parser::parse_tag(std::string_view str, Tag& out_val) {
        auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), out_val);
        if (ec != std::errc{} || ptr != str.data() + str.size()) {
            return false;
        }
        return true;
    }   

    void Parser::parse(std::string_view msg_frame, GenericMessage<GenericFieldView>& out_msg, std::vector<ParseErrorInfo>& out_errs) {
        out_msg.clear();
        out_errs.clear();

        // start with checksum validation
        auto checksum_pos = msg_frame.rfind("10=");
        if (checksum_pos == std::string_view::npos) {
            out_errs.push_back({10, ParseError::Failed_checksum});
            return;
        }

        std::uint16_t checksum;
        auto checksum_str = msg_frame.substr(checksum_pos + 3, 3);
        auto [ptr, ec] = std::from_chars(checksum_str.data(), checksum_str.data() + checksum_str.size(), checksum);
        if (ec != std::errc{} || ptr != checksum_str.data() + checksum_str.size()) {
            out_errs.push_back({10, ParseError::Failed_checksum});
            return;
        }

        std::uint64_t computed_checksum = 0;
        for (std::size_t i = 0; i < checksum_pos; ++i) {
            computed_checksum += static_cast<std::uint8_t>(msg_frame[i]);
        }
        computed_checksum %= 256;
        if (computed_checksum != checksum) {
            out_errs.push_back({10, ParseError::Failed_checksum});
            return;
        }

        // parse fields
        std::size_t pos = 0;
        while (pos < checksum_pos) {
            // assuming pos points to the start of a tag
            auto equal_pos = msg_frame.find('=', pos);
            if (equal_pos == std::string_view::npos || equal_pos - pos > MAX_TAG_SIZE) {
                out_errs.push_back({Parser::undefined_tag, ParseError::MalformedTag});
                return;
            }

            Tag tag;
            if (!parse_tag(msg_frame.substr(pos, equal_pos - pos), tag)) {
                out_errs.push_back({Parser::undefined_tag, ParseError::MalformedTag});
                return;
            }

            pos = equal_pos + 1;
            auto soh_pos = msg_frame.find('\x01', pos);
            if (soh_pos == std::string_view::npos) {
                out_errs.push_back({tag, ParseError::Missing_soh});
                return;
            }

            std::string_view value = msg_frame.substr(pos, soh_pos - pos);
            out_msg.push_back({value, tag});
            pos = soh_pos + 1;
        }   
        out_msg.push_back({checksum_str, 10});
    }   
}