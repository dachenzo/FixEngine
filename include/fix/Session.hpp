#pragma once
#include <memory>
#include <optional>
#include <fix/IConnection.hpp>
#include <fix/definitions.hpp>
#include <fix/MessageStore.hpp>
#include <fix/Message.hpp>
#include <fix/Application.hpp>
#include <fix/ITimer.hpp>
#include <fix/Codec.hpp>
#include <fix/Parser.hpp>


namespace Fix {

    

    struct Session: public std::enable_shared_from_this<Fix::Session> {
        // ctor/dtor
        Session(Fix::SessionID id,
                Fix::Role role,
                std::unique_ptr<IConnection> conn,
                Fix::Application& app,
                Fix::ITimerFactory& timers);

        ~Session();

        // lifecycle
        void start();                       // open/logon loop
        void stop();                        // send logout + cleanup

        // // I/O callbacks (called by your event loop / Connection)
        // void onBytes(std::string_view chunk);
        // void onWritable();

        Fix::SessionID& get_session_id() noexcept;

        // // timer callbacks (heartbeat, test‐req, logout)
        // void onTimer(Fix::TimerType which);

        // // client/API
        // void sendAppMessage(const Fix::Message&);

        private:
            // core dispatch
            void dispatch(const Fix::Message& msg);
            // void checkInboundSeq(const Fix::Message&);

            // // FIX admin sends
            // void sendLogon();
            // void sendLogout(const std::string& reason);
            // void sendHeartbeat(const std::optional<std::string>& testReqId = {});
            // void sendTestRequest(const std::string& testReqId);
            // void sendResendRequest(std::size_t beginSeqNo, std::size_t endSeqNo);
            // void sendSequenceResetGapFill(std::size_t newSeqNo);
            // void resendBufferedMessages(std::size_t beginSeqNo, std::size_t endSeqNo);

            // // FIX admin handlers
            // void handleLogon(const Fix::Message&);
            // void handleLogout(const Fix::Message&);
            // void handleHeartbeat(const Fix::Message&);
            // void handleTestRequest(const Fix::Message&);
            // void handleResendRequest(const Fix::Message&);
            // void handleSequenceReset(const Fix::Message&);

            // // serialize, stamp header/trailer, persist & write
            // void stampAndSend(Fix::Message&);

            void do_read();

            std::vector<char> buff_;
            std::shared_ptr<IConnection> conn_;
            Fix::Parser parser_;
            Fix::Role role_;
            Fix::SessionID id_;
            Fix::Application& app_;
            Fix::ITimerFactory& timers_;
            Fix::MessageStore store_;



    };
}