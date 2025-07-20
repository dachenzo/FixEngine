#include <utility>
#include <string>
#include <string_view>
#include <vector>
#include <optional>

namespace Fix {


    struct Field{
        int tag;
        std::string value;
        std::string_view raw_bytes;
    };


    using Message = std::vector<Fix::Field>;



    struct MessageBuilder {

        void add(Fix::Field& field) {
            if (ready_) {} //throw an error
            if (field.tag == 9) {
                body_length_ = std::stoi(field.value);
                body_length_count_ = 0;
            }

            if (field.tag == 10) {
                std::size_t checksum = std::stoi(field.value);
                if (checksum_count_ % 256 != checksum) {}
                if (body_length_count_ != body_length_) {}
                ready_ = true;
            }


            for (unsigned char c: field.raw_bytes) {checksum_count_ += c;}
            body_length_count_ += field.raw_bytes.size();
            message_.push_back(field);
        }

        bool ready() {return ready_;}

        Fix::Message get() {
            Fix::Message result = std::move(message_);
            reset_state_();
            return result;
        }

        private:
        std::size_t body_length_;
        std::size_t body_length_count_;
        std::size_t checksum_count_;
        Fix::Message message_;
        bool ready_{false};

        void reset_state_() {
            message_ = Fix::Message();
            message_.reserve(32);
            checksum_count_ = 0;
            body_length_count_ = 0;
            body_length_ = 0;
            ready_ = false;
        }
    };

};