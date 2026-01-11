#include <utility>
#include <string>
#include <string_view>
#include <iostream>
#include <vector>
#include <optional>
#include <span>
#include <unordered_map>
#include <charconv>
#include <fix/core/Message.hpp>


namespace Fix {
    ValidMessage make_valid_message(const Message::GenericMessage& msg) {
        HeaderCache cache;
        for (const auto& field : msg) {
            switch (field.tag) {
                case 35: // MsgType
                    cache.slots[static_cast<size_t>(CacheSlot::MsgType)] = &field.value;
                    break;
                case 49: // SenderCompID
                    cache.slots[static_cast<size_t>(CacheSlot::SenderCompID)] = &field.value;
                    break;
                case 56: // TargetCompID
                    cache.slots[static_cast<size_t>(CacheSlot::TargetCompID)] = &field.value;
                    break;
                case 52: // SendingTime
                    cache.slots[static_cast<size_t>(CacheSlot::SendingTime)] = &field.value;
                    break;
                case 43: // PossDupFlag
                    cache.slots[static_cast<size_t>(CacheSlot::PossDupFlag)] = &field.value;
                    break;
                case 122: // OrigSendingTime
                    cache.slots[static_cast<size_t>(CacheSlot::OrigSendingTime)] = &field.value;
                    break;
                case 112: // TestReqID
                    cache.slots[static_cast<size_t>(CacheSlot::TestReqID)] = &field.value;
                    break;
                case 34: // MsgSeqNum
                    {
                        cache.slots[static_cast<size_t>(CacheSlot::MsgSeqNum)] = &field.value;
                        auto [ptr, ec] = std::from_chars(field.value.data(), field.value.data() + field.value.size(), cache.msg_seq_num);
                        if (ec == std::errc() && ptr == field.value.data() + field.value.size()) {
                            cache.has_msg_seq_num = true;
                        }
                    }
                    break;
                default:
                    break;
            }
        }
        return ValidMessage{msg, cache};
    }
};

