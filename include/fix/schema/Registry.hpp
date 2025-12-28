#pragma once
#include <unordered_map>
#include <array>
#include <algorithm>
#include <fix/schema/MessageSchema.hpp>
#include <fix/message/MessageList.hpp>


namespace Fix::Schema {

    template <MessageBody body>
    inline constexpr MessageSchema make_schema() {
        return {
            body::MsgType,
            body::Schema,
            body::SchemaSize
        };
    }

    template <MessageBody... bodies>
    constexpr auto make_message_registry() {
        auto tmp = std::array<MessageSchema, sizeof...(bodies)>{make_schema<bodies>()...};
                
        return tmp;
    }

    template <std::size_t T>
    constexpr auto make_sorted_registry_indices(const std::array<MessageSchema, T>& arr) {
        std::array<std::size_t, T> indices{};
        for (std::size_t i = 0; i < T; i++) {
            indices[i] = i;
        }

        std::sort(indices.begin(), indices.end(), [&arr](std::size_t a, std::size_t b) {
            return arr[a].msg_type < arr[b].msg_type;
        });

        return indices;
    }



    struct Registry {
        static inline constexpr auto message_schemas = make_message_registry<Message::Logon>();
        static inline constexpr auto sorted_indices = make_sorted_registry_indices<message_schemas.size()>(message_schemas);
        

        constexpr const MessageSchema* get(std::string_view message_type) const {
            std::size_t left = 0;
            std::size_t right = sorted_indices.size(); // [left, right)

            while (left < right) {
                std::size_t mid = left + (right - left) / 2;

                const std::size_t idx = sorted_indices[mid];
                const auto mid_key = message_schemas[idx].msg_type;

                if (mid_key < message_type) {
                    left = mid + 1;
                } else {
                    right = mid; // mid_key >= message_type
                }
            }

            // left is the first position where mid_key >= message_type
            if (left < sorted_indices.size()) {
                const std::size_t idx = sorted_indices[left];
                if (message_schemas[idx].msg_type == message_type) {
                    return &message_schemas[idx];
                }
            }
            return nullptr;
        }


    };

    
    



}