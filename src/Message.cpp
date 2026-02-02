#include <utility>
#include <string>
#include <string_view>
#include <charconv>
#include <fix/core/Message.hpp>


namespace Fix {
    ValidMessageView make_valid_message_view(const GenericMessage<GenericFieldView>& msg) {
        HeaderCache<std::string_view> cache;
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
        return ValidMessageView{msg, cache};
    }


    ValidMessage make_valid_message(const GenericMessage<GenericFieldView>& msg) {
        GenericMessage<GenericField> new_msg;
        new_msg.reserve(msg.size());
        HeaderCache<std::string> cache;
        for (const auto& field : msg) {
            new_msg.push_back(GenericField{std::string(field.value), field.tag});
            switch (field.tag) {
                case 35: // MsgType
                    cache.slots[static_cast<size_t>(CacheSlot::MsgType)] = &new_msg.back().value;
                    break;
                case 49: // SenderCompID
                    cache.slots[static_cast<size_t>(CacheSlot::SenderCompID)] = &new_msg.back().value;
                    break;
                case 56: // TargetCompID
                    cache.slots[static_cast<size_t>(CacheSlot::TargetCompID)] = &new_msg.back().value;
                    break;
                case 52: // SendingTime
                    cache.slots[static_cast<size_t>(CacheSlot::SendingTime)] = &new_msg.back().value;
                    break;
                case 43: // PossDupFlag
                    cache.slots[static_cast<size_t>(CacheSlot::PossDupFlag)] = &new_msg.back().value;
                    break;
                case 122: // OrigSendingTime
                    cache.slots[static_cast<size_t>(CacheSlot::OrigSendingTime)] = &new_msg.back().value;
                    break;
                case 112: // TestReqID
                    cache.slots[static_cast<size_t>(CacheSlot::TestReqID)] = &new_msg.back().value;
                    break;
                case 34: // MsgSeqNum
                    {
                        cache.slots[static_cast<size_t>(CacheSlot::MsgSeqNum)] = &new_msg.back().value;
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
        return ValidMessage{std::move(new_msg), std::move(cache)};
    }
};

