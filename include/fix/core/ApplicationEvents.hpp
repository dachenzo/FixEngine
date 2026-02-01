#pragma once 
#include <functional>
#include <fix/core/Message.hpp>
#include <fix/message/GenericMessage.hpp>
#include <fix/core/definitions.hpp>



namespace Fix {
    
    struct OutBoundAppMsg {
        GenericMessage<GenericField> message;
        SessionID session_id;
        MsgType msg_type;
    };

    struct InBoundAppEvent {
        ValidMessage valid_message;
        SessionID session_id;
    };

    using AppSink = std::function<void(InBoundAppEvent&&)>;

    
}
