#pragma once
#include <charconv>
#include <vector>
#include <tuple>
#include <fix/core/utils.hpp>
#include <fix/error/ValidatorErrors.hpp>
#include <fix/schema/Field.hpp>
#include <fix/message/GenericMessage.hpp>
#include <fix/message/Header.hpp>
#include <fix/core/definitions.hpp>
#include <fix/schema/Registry.hpp>


namespace Fix {
    using ValidatorResult = std::vector<std::tuple<Error::Validator, std::size_t>>;

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

        void set(Fix::Message::GenericMessage::const_iterator it, const Fix::Message::GenericMessage::const_iterator& begin) {
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
        // All functions will eventually return Validaro Result;

        ValidatorResult validate_message(const Message::GenericMessage& message, std::string& expected_message_type);

        void validate_header_(const Message::GenericMessage& message, std::string& expected_message_type, ValidatorResult& validres);

        void validate_trailer_(const Message::GenericMessage& message, ValidatorResult& validres);


        void validate_fields_(const Message::GenericMessage& message, const Schema::FieldSchema* schema, std::size_t schema_size, ValidatorResult& validres);

        void validate_groups_(const std::size_t groupcnt, const Message::GenericMessage& message, const Schema::FieldSchema* groupfield, int& curr_idx, ValidatorResult& validres);

        bool validate_type_(const std::string& value, Fix::Schema::FieldType type);

        private:
        TagScratch tagscratch_;
        Schema::Registry registry_;

    };
}