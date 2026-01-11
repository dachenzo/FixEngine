#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <span>
#include <optional>
#include <cstdlib>
#include <cstdint>
#include <unordered_map>
#include <fix/message/GenericMessage.hpp>

namespace Fix {


    enum class CacheSlot : uint8_t {
        MsgType,
        SenderCompID,
        TargetCompID,
        SendingTime,
        MsgSeqNum,
        PossDupFlag,
        OrigSendingTime,
        TestReqID,
        COUNT
    };

    struct HeaderCache {
        const std::string* slots[static_cast<size_t>(CacheSlot::COUNT)] = {nullptr};
        uint64_t msg_seq_num = 0;
        bool has_msg_seq_num = false;
    };



    struct ValidMessage {
        const Message::GenericMessage message_;
        const HeaderCache header_cache_;
    };

    ValidMessage make_valid_message(const Message::GenericMessage& msg);



};