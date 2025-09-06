#pragma once
#include <memory>
#include <optional>
#include <deque>
#include <fix/SeqProvider.hpp>
#include <fix/Clock.hpp>
#include <fix/Serializer.hpp>
#include <fix/IConnection.hpp>
#include <fix/definitions.hpp>
#include <fix/MessageStore.hpp>
#include <fix/Message.hpp>
#include <fix/MessageFactory.hpp>
#include <fix/Application.hpp>
#include <fix/ITimer.hpp>
#include <fix/Codec.hpp>
#include <fix/Parser.hpp>


namespace Fix {

    enum class SessionState {
        AWAITING_LOGON,
        LOGON_SENT,
        ACTIVE,
        DISCONNECTED,
    };

    

    struct Session: public std::enable_shared_from_this<Fix::Session> {
        // ctor/dtor
        Session(Fix::SessionID id,
                Fix::Role role,
                Fix::Application& app,
                Fix::ITimerFactory& timers,
                Fix::SessionParameters params
            );

        ~Session();

        // lifecycle
        void start();                       // open/logon loop
        void stop();                        // send logout + cleanup

        // // I/O callbacks (called by your event loop / Connection)
        // void onBytes(std::string_view chunk);
        // void onWritable();

        Fix::SessionID& get_session_id() noexcept;

        void set_connection(std::shared_ptr<Fix::IConnection> conn);

        // // timer callbacks (heartbeat, test‐req, logout)
        // void onTimer(Fix::TimerType which);

        // // client/API
        // void sendAppMessage(const Fix::Message&);

        private:
            // core dispatch
            void dispatch(Fix::Message& msg);
            // void checkInboundSeq(const Fix::Message&);

            // // FIX admin sends
            void send_logon();
            // void sendLogout(const std::string& reason);
            // void sendHeartbeat(const std::optional<std::string>& testReqId = {});
            // void sendTestRequest(const std::string& testReqId);
            // void sendResendRequest(std::size_t beginSeqNo, std::size_t endSeqNo);
            // void sendSequenceResetGapFill(std::size_t newSeqNo);
            // void resendBufferedMessages(std::size_t beginSeqNo, std::size_t endSeqNo);

            // FIX admin handlers
            void handle_logon(const Fix::Message&);
            void handle_logout(const Fix::Message&);
            void handle_heartbeat(const Fix::Message&);
            void handle_test_request(const Fix::Message&);
            void handle_resend_request(const Fix::Message&);
            void handle_sequence_reset(const Fix::Message&);

            void send_message_(Fix::Message& msg);

            void send_bytes_(std::string msg_wire);
            
            void do_read();

            void do_write();
            struct PendingWrite { std::string data; std::size_t sent = 0; };
            std::deque<PendingWrite> write_q_;
            bool write_inflight_ = false;
            std::vector<char> buff_;
            std::shared_ptr<IConnection> conn_;
            Fix::Parser parser_;
            Fix::Serializer serializer_;
            Fix::Role role_;
            Fix::SessionID id_;
            Fix::Application& app_;
            Fix::ITimerFactory& timers_;
            Fix::MessageStore store_;
            Fix::SessionState state_;
            Fix::SessionParameters params_;
            Fix::SeqProvider seq_provider_;
            Fix::Clock clock_;
            Fix::MessageFactory msg_factory_;
            



    };
}