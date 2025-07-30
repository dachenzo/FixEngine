#pragma once
#include <memory>
#include <optional>
#include "IConnection.hpp"
#include "definitions.hpp"
#include "MessageStore.hpp"
#include "Message.hpp"
#include "Application.hpp"
#include "ITimer.hpp"
#include "Codec.hpp"
#include "Parser.hpp"


namespace Fix {
    struct Session: public std::enable_shared_from_this<Fix::Session> {
        // ctor/dtor
        Session(Fix::SessionID id,
                Fix::Role role,
                std::shared_ptr<IConnection> conn,
                Fix::Codec& codec,
                Fix::MessageStore& store,
                Fix::Application& app,
                Fix::ITimerFactory& timers);
        ~Session();

        // lifecycle
        void start();                       // open/logon loop
        void stop();                        // send logout + cleanup

        // I/O callbacks (called by your event loop / Connection)
        void onBytes(std::string_view chunk);
        void onWritable();

        // timer callbacks (heartbeat, test‐req, logout)
        void onTimer(Fix::TimerType which);

        // client/API
        void sendAppMessage(const Fix::Message&);

        private:
            // core dispatch
            void dispatch(const Fix::Message&);
            void checkInboundSeq(const Fix::Message&);

            // FIX admin sends
            void sendLogon();
            void sendLogout(const std::string& reason);
            void sendHeartbeat(const std::optional<std::string>& testReqId = {});
            void sendTestRequest(const std::string& testReqId);
            void sendResendRequest(std::size_t beginSeqNo, std::size_t endSeqNo);
            void sendSequenceResetGapFill(std::size_t newSeqNo);
            void resendBufferedMessages(std::size_t beginSeqNo, std::size_t endSeqNo);

            // FIX admin handlers
            void handleLogon(const Fix::Message&);
            void handleLogout(const Fix::Message&);
            void handleHeartbeat(const Fix::Message&);
            void handleTestRequest(const Fix::Message&);
            void handleResendRequest(const Fix::Message&);
            void handleSequenceReset(const Fix::Message&);

            // serialize, stamp header/trailer, persist & write
            void stampAndSend(Fix::Message&);

            void do_read() {}

            std::vector<char> buff_;
            std::shared_ptr<IConnection> conn_;
            Fix::Parser parser_;


    };
}