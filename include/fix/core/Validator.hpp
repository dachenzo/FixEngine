#pragma once

#include <vector>
#include <array>
#include <cstring>
#include <fix/core/utils.hpp>
#include <fix/error/ValidatorErrors.hpp>
#include <fix/schema/Field.hpp>
#include <fix/message/GenericMessage.hpp>
#include <fix/message/Header.hpp>
#include <fix/core/definitions.hpp>
#include <fix/schema/Registry.hpp>
#include <fix/error/Severity.hpp>
#include <fix/message/Trailer.hpp>
#include <fix/core/Message.hpp>


namespace Fix {

    struct ErrorString {
        // forces fixed size to avoid allocations
        std::array<char, 32> data;
        std::size_t size{0};

        ErrorString() = default;
        ErrorString(const char* str) {
            size = std::strlen(str);
            if (size > data.size()) size = data.size();
            std::memcpy(data.data(), str, size);
        }
        ErrorString(std::string_view sv) {
            size = sv.size();
            if (size > data.size()) size = data.size();
            std::memcpy(data.data(), sv.data(), size);
        }
        operator std::string_view() const {
            return std::string_view{data.data(), size};
        }
    };


    struct ValidatorErrorContext {
        ErrorString info;
        Tag tag;
        Error::Validator code;

    };

    struct ValidatorResult {
        static constexpr const std::size_t errors_reserve = 64;
        Error::Severity severity{Error::Severity::Moderate};
        std::vector<ValidatorErrorContext> errors;

        ValidatorResult() {
            errors.reserve(errors_reserve);
        }

        void clear() {
            severity = Error::Severity::Moderate;
            errors.clear();
        }
    }; 
    

    struct TagScratch {
        public:
        TagScratch() {
            data_.reserve(1024);
        }
        ~TagScratch() = default;

        void ensure_bits(std::size_t size) {
            if (data_.size() < (size >> 6)+1) {
                data_.resize((size >> 6)+1);
            }

        }

        void clear() {
            for (auto& c: data_) c = 0;
        }

        bool full(std::size_t size) {
            std::size_t max = ~uint64_t{0};
            std::size_t q = size >> 6;
            std::size_t r = size & 63; // remainder bits in the last 64-bit block

            for (int i = 0; i < q; i++) {
                if (data_[i] != max) return false;
            }

            if (r != 0) {
                uint64_t mask = (1UL << r) - 1;
                if ((data_[q] & mask) != mask) return false;
            }

            return true;
        }

        void set(std::size_t index) {
            data_[index >> 6] |= (1UL << (index & 63));
        }

        void set(Fix::GenericMessage<GenericFieldView>::const_iterator it, const Fix::GenericMessage<GenericFieldView>::const_iterator& begin) {
            std::size_t index = std::distance(begin, it);
            data_[index >> 6] |= (1UL << (index & 63));
        }

        bool get(std::size_t index) {
            return data_[index >> 6] & (1UL << (index & 63));
        }

        private:
        std::vector<uint64_t> data_;
    };
    

    struct Validator
    {
        // All functions will eventually return Validator Result;

        ValidatorResult& validate_message(const ValidMessage& message, const Fix::SessionParameters& params);

        void validate_header_(const ValidMessage& message, const std::string_view expected_message_type, ValidatorResult& validres, const Fix::SessionParameters& params);

        void validate_trailer_(const ValidMessage& message, ValidatorResult& validres);


        void validate_fields_(const ValidMessage& message, const Schema::FieldSchema* schema, std::size_t schema_size, ValidatorResult& validres);

        void validate_groups_(const std::size_t groupcnt, const ValidMessage& message, const Schema::FieldSchema* groupfield, int& curr_idx, ValidatorResult& validres);

        bool validate_type_(const std::string_view value, Fix::Schema::FieldType type);

        private:
        ValidatorResult results_;
        TagScratch tagscratch_;
        Schema::Registry registry_;


    };
}