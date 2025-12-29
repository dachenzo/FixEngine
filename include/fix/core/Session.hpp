#pragma once
#include <memory>
#include <optional>
#include <deque>
#include <fix/core/SeqProvider.hpp>
#include <fix/core/Clock.hpp>
#include <fix/core/Serializer.hpp>
#include <fix/core/IConnection.hpp>
#include <fix/core/definitions.hpp>
#include <fix/core/MessageStore.hpp>
#include <fix/core/Message.hpp>
#include <fix/core/MessageFactory.hpp>
#include <fix/core/Application.hpp>
#include <fix/core/ITimer.hpp>
#include <fix/core/Codec.hpp>
#include <fix/core/Validator.hpp>
#include <fix/core/Parser.hpp>
#include <fix/log/LogCore.hpp>
#include <fix/log/SessionLogger.hpp>
#include <fix/message/GenericMessage.hpp>


namespace Fix {

    enum class SessionState {
        AWAITING_LOGON,
        LOGON_RECEIVED,
        LOGON_SENT,
        ACTIVE,
        DISCONNECTED,
    };



    struct Session: public std::enable_shared_from_this<Fix::Session> {

        static constexpr const int logon_response_timeout = 10;
        // ctor/dtor
        Session(Fix::SessionID id,
                Fix::Role role,
                Fix::Application& app,
                Fix::ITimerFactory& timers,
                Fix::SessionParameters params,
                Fix::Log::LogCore& log_core
            );

        ~Session();

        // lifecycle
        void start();                       // open/logon loop
        void stop();                        // send logout + cleanup

        // // I/O callbacks (called by your event loop / Connection)
        // void onBytes(std::string_view chunk);
        // void onWritable();

        Fix::SessionID get_session_id()const noexcept;

        void set_connection(std::shared_ptr<Fix::IConnection> conn);

        std::string readable_id() const noexcept;

        Log::SessionLogger& logger() ;

        // // timer callbacks (heartbeat, test‐req, logout)
        // void onTimer(Fix::TimerType which);

        // // client/API
        // void sendAppMessage(const Fix::ValidMessage&);

        private:
            // core dispatch
            void dispatch(Message::GenericMessage& msg) ;
            // void checkInboundSeq(const Fix::ValidMessage&);

            // // FIX admin sends
            void send_logon();
            // void sendLogout(const std::string& reason);
            // void sendHeartbeat(const std::optional<std::string>& testReqId = {});
            // void sendTestRequest(const std::string& testReqId);
            // void sendResendRequest(std::size_t beginSeqNo, std::size_t endSeqNo);
            // void sendSequenceResetGapFill(std::size_t newSeqNo);
            // void resendBufferedMessages(std::size_t beginSeqNo, std::size_t endSeqNo);

            // FIX admin handlers
            void handle_logon(const Fix::ValidMessage&);
            void handle_logout(const Fix::ValidMessage&);
            void handle_heartbeat(const Fix::ValidMessage&);
            void handle_test_request(const Fix::ValidMessage&);
            void handle_resend_request(const Fix::ValidMessage&);
            void handle_sequence_reset(const Fix::ValidMessage&);

            void send_message_(Fix::ValidMessage& msg);

            void send_bytes_(std::string msg_wire);

            void schedule_logon_timeout_(Fix::SessionState expected_state);
            
            void do_read();

            



            void do_write();


            struct PendingWrite { std::string data; std::size_t sent = 0; };

            boost::asio::strand<boost::asio::any_io_executor> exec_{boost::asio::system_executor()};
            std::deque<PendingWrite> write_q_;
            bool stopped_ = false;
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
            Fix::Log::SessionLogger logger_;
            boost::asio::steady_timer logon_timer_;
            boost::asio::steady_timer inbound_timer_;
            boost::asio::steady_timer heartbeat_timer_;
            Fix::Validator validator_;

    };
}