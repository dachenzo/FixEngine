#include <fix/MessageBuilder.hpp>
#include <fix/utils.hpp>


namespace Fix {
    void MessageBuilder::add(Fix::RawField& raw_field) {
        // Update checksum with *previous* bytes only; do not include tag 10.
        // For any field except 10, add raw bytes to checksum.
        if (raw_field.tag != 10) {
            for (unsigned char c: raw_field.raw_bytes) { checksum_count_ += c;  }
            checksum_count_ += static_cast<unsigned char>('\x01');
        
        }

        // Handle BodyLength
        if (raw_field.tag == 9) {
            std::size_t body_len;
            if (!Utils::parse_int(raw_field.value, body_len)) {
                error_.errs.push_back(Error::Parse::Wrong_body_length);
                error_.sev = Error::Severity::Fatal;
                return;
            }
            body_length_ = body_len; 
            body_length_count_ = 0; // start counting *after* 9-field
        } else if (raw_field.tag != 10) {
            body_length_count_ += raw_field.raw_bytes.size()+1; // count everything between 9 and 10
        }

        // Handle CheckSum
        if (raw_field.tag == 10) {

            std::size_t checksum = 0;
            if (!Utils::parse_int(raw_field.value, checksum)) {
                error_.errs.push_back(Error::Parse::Failed_checksum);
                error_.sev = Error::Severity::Fatal;
                return;
            }
            const bool checksum_ok = (checksum_count_ % 256) == checksum;
            const bool body_ok  = (body_length_count_ == body_length_);
            if (!checksum_ok) error_.errs.push_back(Error::Parse::Failed_checksum);
            if (!body_ok) error_.errs.push_back(Error::Parse::Wrong_body_length);
            ready_ = checksum_ok && body_ok;  
             // <-- only ready if both pass

            if (!ready_) error_.sev = Error::Severity::Fatal;
            
        }

        // Store field in the message
        bool res = message_.add(Fix::Field{raw_field.tag, std::move(raw_field.value)});
        if (!res && error_.sev == Error::Severity::NA) {
            //  ignore this if theres an error of higher severity
            error_.errs.push_back(Error::Parse::Duplicate_tag);
            error_.sev = Error::Severity::Moderate;

        }
    }

    BuildResult MessageBuilder::ready() const{
        return {!error_.errs.empty(), ready_};
    }

    Fix::Message MessageBuilder::get()  {
        Fix::Message result = std::move(message_);
        reset_state();
        return result;
    }

    MessageErrorView MessageBuilder::get_error_state() const {
        return {std::span(error_.errs), error_.sev};
    }

    

    void MessageBuilder::reset_state() {
        message_ = Fix::Message();
        checksum_count_ = 0;
        body_length_count_ = 0;
        body_length_ = 0;
        ready_ = false;
        error_.errs.clear();
        error_.sev = Error::Severity::NA;

    }
}