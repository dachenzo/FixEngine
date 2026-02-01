#pragma once
#include <memory>
#include <deque>
#include <fix/core/Arena.hpp>
#include <fix/core/SeqProvider.hpp>
#include <fix/core/Clock.hpp>
#include <fix/core/Framer.hpp>
#include <fix/core/WireWriter.hpp>
#include <fix/core/Serializer.hpp>
#include <fix/core/IConnection.hpp>
#include <fix/core/definitions.hpp>
#include <fix/core/MessageStore.hpp>
#include <fix/core/Message.hpp>
#include <fix/core/RecoveryCache.hpp>
#include <fix/core/MessageFactory.hpp>
#include <fix/core/Application.hpp>
#include <fix/core/ITimer.hpp>
#include <fix/core/Codec.hpp>
#include <fix/core/Validator.hpp>
#include <fix/core/WireWriter.hpp>
#include <fix/core/Parser.hpp>
#include <fix/log/LogCore.hpp>
#include <fix/log/SessionLogger.hpp>
#include <fix/message/GenericMessage.hpp>
#include <fix/core/ApplicationEvents.hpp>
#include <string_view>


namespace Fix {

    enum class SessionState {
        AWAITING_LOGON,
        AWAITING_LOGOUT,
        LOGON_RECEIVED,
        LOGON_SENT,
        RECOVERING_RESEND,
        ACTIVE,
        DISCONNECTED,
    };

    enum class StartMode {
        NORMAL,
        RECONNECT
    };

    using ReconnectCallback = std::function<void(const Fix::SessionID)>;



    struct Session: public std::enable_shared_from_this<Fix::Session> {

        static constexpr const int logon_response_timeout = 10;
        static constexpr const int logout_response_timeout = 30;
        // ctor/dtor
        Session(Fix::SessionID id,
                Fix::Role role,
                Fix::Application& app,
                Fix::ITimerFactory& timers,
                Fix::SessionParameters params,
                Fix::Log::LogCore& log_core,
                boost::asio::io_context& io_context,
                ReconnectCallback reconnect_callback
            );

        ~Session();
        Session(const Session&) = delete;
        Session& operator=(const Session&) = delete;   
        Session(Session&&) = delete;
        Session& operator=(Session&&) = delete; 


        // lifecycle
        void start(StartMode mode);                       // open/logon loop
        void stop();                        // send logout + cleanup


        Fix::SessionID get_session_id()const noexcept;

        void set_connection(std::shared_ptr<Fix::IConnection> conn);

        std::string readable_id() const noexcept;

        Log::SessionLogger& logger() ;


        //app callbacks
        void send_from_app(OutBoundAppMsg&& msg);

        private:

            //lifecycle helpers
            void start_normal_();
            void start_after_reconnect_();
            void stop_();

            // core dispatch
            void dispatch(const GenericMessage<GenericFieldView>& msg, std::string_view raw_msg);
            void process_wire_message_(std::string_view msg_wire);
            void drain_recovery_cache_();

            
            // core state
            void schedule_heartbeat_();
            void schedule_logon_timeout_(Fix::SessionState expected_state);
            void schedule_test_request_timeout_();
            void stop_with_logout(const std::string& reason);

            // // FIX admin sends 
            void send_logon(bool reset_seq_nums);
            void send_reject(std::size_t ref_seq_num, uint32_t reason, Tag tag, std::string_view text);
            void send_logout(const std::string& reason);
            void send_heartbeat(const std::string_view testReqId = {});
            void send_test_request(const std::string& testReqId);
            void send_resend_request(std::size_t beginSeqNo, std::size_t endSeqNo);
            void send_sequence_reset(std::size_t newSeqNo, bool gapfill);
           

            // FIX admin handlers
            void handle_logon(const Fix::ValidMessage& message);
            void handle_logout(const Fix::ValidMessage& message);
            void handle_heartbeat(const Fix::ValidMessage& message);
            void handle_test_request(const Fix::ValidMessage& message);
            void handle_resend_request(const Fix::ValidMessage& message);
            void handle_sequence_reset(const Fix::ValidMessage& message);


            //core IO
            void send_message_(std::string_view msg_wire, bool is_resend = false);
            void send_bytes_(Fix::WireWriter handle);            
            void do_read();
            void do_write();
            void on_transport_down_();


            //validation
            void validate_heartbeat_int(std::string_view incoming_value, bool is_initiator);
            bool is_app_message_type_(std::string_view msg_type) const noexcept;


            


            struct PendingWrite { 
                Fix::WireWriter data; 
                std::size_t sent = 0; 
            };

            Fix::RecoveryCache recovery_cache_;
            boost::asio::strand<boost::asio::any_io_executor> exec_;
            Fix::Arena arena_;
            Fix::SessionParameters params_;
            std::deque<PendingWrite> write_q_;
            std::vector<char> buff_;
            std::shared_ptr<IConnection> conn_;
            Fix::Framer framer_;
            Fix::ParserContext parser_ctx_;
            Fix::Parser parser_;
            Fix::SessionID id_;
            Fix::MessageStore store_;
            Fix::SessionState state_;
            Fix::SeqProvider seq_provider_;
            ReconnectCallback reconnect_callback_;
            Fix::Clock clock_;
            Fix::MessageFactory<Fix::Clock> msg_factory_;
            Fix::Log::SessionLogger logger_;
            boost::asio::steady_timer logon_timer_;
            boost::asio::steady_timer inbound_timer_;
            boost::asio::steady_timer heartbeat_timer_;
            boost::asio::steady_timer logout_timer_;
            Fix::Validator validator_;
            Fix::Application& app_;
            Fix::ITimerFactory& timers_;
            std::uint64_t test_req_id_ = 0;
            bool stopped_ = false;  // user requested stop / session shutting down
            bool write_inflight_ = false;
            bool awaiting_test_request_response_ = false;
            bool reconnecting_ = false;
            Fix::Role role_;
            

    };
}