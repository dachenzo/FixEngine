#pragma once
#include <string>
#include <string_view>
#include <cstdlib>
#include <cstdint>
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

    using SeqNum = uint64_t;

    template<typename StringType>
    struct HeaderCache {
        const StringType* slots[static_cast<size_t>(CacheSlot::COUNT)] = {nullptr};
        SeqNum msg_seq_num = 0;
        bool has_msg_seq_num = false;
    };



    struct ValidMessageView {
        ValidMessageView(const GenericMessage<GenericFieldView>& msg, const HeaderCache<std::string_view>& cache):
        message_(msg),
        header_cache_(cache)
        {}
        ValidMessageView(ValidMessageView const&) = delete;
        ValidMessageView& operator=(ValidMessageView const&) = delete;
        ValidMessageView(ValidMessageView&&) = delete;
        ValidMessageView& operator=(ValidMessageView&&) = delete;
        
        const GenericMessage<GenericFieldView>& message_;
        const HeaderCache<std::string_view> header_cache_;
    };

    struct ValidMessage {
        GenericMessage<GenericField> message_;
        HeaderCache<std::string> header_cache_;
    };

    ValidMessageView make_valid_message_view(const GenericMessage<GenericFieldView>& msg);

    ValidMessage make_valid_message(const GenericMessage<GenericFieldView>& msg);


  





};