#include <fix/core/MessageFactory.hpp>
#include <charconv>
#include <iostream>



namespace Fix {

    MessageFactory::MessageFactory( SessionParameters& params, 
                                    SeqProvider& seq_provider, 
                                    Clock& clock): scratch_{}, params_{params}, seq_provider_{seq_provider}, clock_{clock} {
        
    }

    void MessageFactory::stamp_header_(std::string type) {
        scratch_.reset();
        scratch_.add_field(8, params_.fix_version);
        scratch_.add_body_length_placeholder();
        scratch_.add_field(35, type);
        scratch_.add_field(34, std::to_string(seq_provider_.next_out()));
        scratch_.add_field(49, params_.sender_comp_id);
        scratch_.add_field(56, params_.target_comp_id);
        scratch_.add_field(52, clock_.now_fix());
    }


    std::string_view MessageFactory::logon(int heartbeat_override, bool echo_reset) {
        stamp_header_("A");
        scratch_.add_field(98, params_.encrypt_method_str);
        scratch_.add_field(141, (echo_reset ? "Y" : "N"));
        scratch_.add_field(108, params_.heart_beat_str);
        stamp_trailer_();
        return scratch_.get_buffer_view();
    }

    void MessageFactory::stamp_trailer_() {
        scratch_.insert_body_length();
        scratch_.insert_checksum();
    }





    

    // Fix::ValidMessage MessageFactory::heart_beat(std::optional<std::string> test_req_id) {
    //     Fix::ValidMessage msg{};
    //     stamp_header_(msg, "0");
    //     if (test_req_id.has_value()) {msg.add({112, test_req_id.value()});}
    //     stamp_trailer_(msg);
    //     return msg;
    // }

    // Fix::ValidMessage MessageFactory::test_request(std::string id) {
    //     Fix::ValidMessage msg{};
    //     stamp_header_(msg, "1");
    //     msg.add({112, id});
    //     stamp_trailer_(msg);
    //     return msg;        

    // }

    // Fix::ValidMessage MessageFactory::resend_request(int begin_seq_no, int end_seq_no) {
    //     Fix::ValidMessage msg{};
    //     stamp_header_(msg, "2");
    //     msg.add({7, std::to_string(begin_seq_no)});
    //     msg.add({16, std::to_string(end_seq_no)});
    //     stamp_trailer_(msg);
    //     return msg;
    // }

    // Fix::ValidMessage MessageFactory::sequence_reset(int newSeqNo, bool gap_fill) {
    //     Fix::ValidMessage msg{};
    //     stamp_header_(msg, "4");
    //     if (gap_fill) msg.add({123, "Y"});   // GapFillFlag
    //     msg.add({36, std::to_string(newSeqNo)}); // NewSeqNo
    //     // If this is going out in a resend stream: add 43=Y and 122=...
    //     stamp_trailer_(msg);
    //     return msg;
    // }

    // Fix::ValidMessage MessageFactory::logout(std::string text) {
    //     Fix::ValidMessage msg{};
    //     stamp_header_(msg, "5");
    //     msg.add({58, text});
    //     stamp_trailer_(msg);
    //     return msg;
    // }



    // void MessageFactory::stamp_trailer_(Fix::ValidMessage& msg) {
    //     auto body_len = compute_body_length_(msg);
    //     msg.set_tag(9, std::to_string(body_len));
    //     auto checksum = compute_check_sum_(msg);
    //     char buf[4];
    //     auto r = std::to_chars(buf, buf+3, checksum);
    //     int len = r.ptr - buf;
    //     std::string cs(3 - len, '0'); // zero-pad
    //     cs.append(buf, r.ptr);
    //     msg.add({10, cs});
        
    // }

    // std::uint64_t MessageFactory::compute_body_length_(Fix::ValidMessage& msg) {
    //     std::uint64_t len = 0;
    //     for (const auto& f : msg.get_fields()) {
    //         if (f.tag == 8 || f.tag == 9 || f.tag == 10) continue;
    //         // bytes: digits(tag) + '=' + value bytes + SOH
    //         len += static_cast<std::uint64_t>(tag_count_(f.tag)) + 1u
    //             + static_cast<std::uint64_t>(f.value.size()) + 1u;
    //     }
    //     return len;
    // }

    // inline std::uint32_t MessageFactory::sum_tag_ascii_(int tag) {
    //     char buf[11]; // enough for 32-bit int
    //     auto r = std::to_chars(buf, buf + sizeof(buf), tag);
    //     uint32_t s = 0;
    //     for (char* p = buf; p != r.ptr; ++p) s += static_cast<unsigned char>(*p);
    //     return s;
    // }

    // inline std::uint32_t MessageFactory::sum_bytes_(std::string_view sv) {
    //     std::uint32_t res = 0;
    //     for (unsigned char c: sv) {
    //         res += c;
    //     }
    //     return res;
    // }   



    // std::uint8_t MessageFactory::compute_check_sum_(Fix::ValidMessage& msg) {
    //     constexpr auto equals = static_cast<unsigned char>('=');

    //     uint64_t res = 0;
    //     for (const auto f: msg.get_fields()) {
    //         if (f.tag == 10) break;
    //         res += sum_tag_ascii_(f.tag);
    //         res += equals;
    //         res += sum_bytes_(f.value);
    //         res += 0x01;
    //     }

    //     return static_cast<uint8_t>(res % 256);
    // }



    // constexpr int MessageFactory::tag_count_(int tag)const noexcept {
    //     if (tag < 10) return 1;
    //     if (tag < 100) return 2;
    //     if (tag < 1000) return 3;
    //     if (tag < 10000) return 4;
    //     if (tag < 100000) return 5;
    //     if (tag < 1000000) return 6;
    //     if (tag < 10000000) return 7;
    //     if (tag < 100000000) return 8;
    //     return 9; 
    // }

}