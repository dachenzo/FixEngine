#include <memory>
#include <string_view>
#include "fix_message.hpp"
#include "fix_parser.hpp"
#include "config/fix_config.hpp"
#include "net/tcp_connection.hpp"

namespace Fix {

    enum class SessionState {
        DISCONECTED,  // No TCP link or awaiting Logon.
        AWAITING_LOGON,
        LOGON_SENT,  // You’ve sent a Logon, awaiting peer’s Logon
        LOGON_RECEIVED,  // You’ve received peer’s Logon, need to send yours.
        ACTIVE, // Both Logons done, message flow is live.
        LOGOUT_SENT, // You initiated a Logout, awaiting peer’s Logout.
        LOGOUT_RECEIVED // Peer initiated Logout, you must reply.
    };

    enum class Role {
        INITIATOR,
        ACCEPTOR
    };

    struct Session {

        Session(std::unique_ptr<TCP::Connection> conn, Fix::Role role):inSeqNum_{1}, outSeqNum_{1}, conn_{std::move(conn)}, parser_{}, role_{role}, state_{Fix::SessionState::DISCONECTED} {}

        void start() {
            if (role_ == Fix::Role::INITIATOR) {
                sendLogon();
            }
            state_ = (role_==Role::INITIATOR ? Fix::SessionState::LOGON_SENT : Fix::SessionState::AWAITING_LOGON);

            while (state_ != Fix::SessionState::DISCONECTED) {
                std::string_view sv = conn_->read();
                std::optional<Fix::Message> message = parser_.parse(sv);

                while (message == std::nullopt) {
                    std::string_view sv = conn_->read();
                    std::optional<Fix::Message> message = parser_.parse(sv);
                }

            }

        }

        void dispatch(Fix::Message& msg) {
            auto msg_type = msg.get(35);
            if (msg_type == "A") return handle_logon(msg);
            if (msg_type == "5") return handle_logout(msg);
            if (msg_type == "0") return handle_heart_beat(msg);
            if (msg_type == "1") return handle_test_request(msg);
            if (msg_type == "2") return handle_resend_message(msg);

            if (state_ != Fix::SessionState::ACTIVE) {
                send_logout("Unexpected MsgType before Active");
            }


            //FixMe: deligate other types to application 
         

        }

        void sendLogon() {}
        
        void send_logout(std::string reason) {}

        void handle_logon(Fix::Message& msg) {}

        void handle_logout(Fix::Message& msg) {}

        void handle_heart_beat(Fix::Message& msg) {}

        void handle_test_request(Fix::Message& msg) {}

        void handle_resend_message(Fix::Message& msg) {}

        void sendAppMesage(const Fix::Message& msg) {}

        void stop() {}

        private:
        Fix::Parser parser_;
        std::unique_ptr<TCP::Connection> conn_;
        std::size_t inSeqNum_;
        std::size_t outSeqNum_;
        Fix::SessionState state_;
        Fix::Role role_;

    };
};