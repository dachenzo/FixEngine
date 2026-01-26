#include <boost/asio.hpp>
#include <boost/asio/dispatch.hpp>
#include <string_view>
#include <string>
#include <charconv>
#include <fix/log/LogEntry.hpp>
#include <fix/error/Network.hpp>
#include <fix/core/Session.hpp>
#include <fix/core/Parser.hpp>
#include <fix/core/utils.hpp>
#include <sys/socket.h>

namespace Fix {

    constexpr const int msg_type_key = 35;
    constexpr const int msg_seq_num_key = 34;
    Session::Session(
                Fix::SessionID id,
                Fix::Role role,
                Fix::Application& app,
                Fix::ITimerFactory& timers,
                Fix::SessionParameters params,
                Fix::Log::LogCore& log_core,
                boost::asio::io_context& io_context
            ):  
                exec_{boost::asio::make_strand(io_context)},
                arena_{},
                recovery_cache_{},
                parser_{},
                framer_{},
                parser_ctx_{},
                app_{app},
                timers_{timers},
                id_{id},
                role_{role},
                store_{},
                state_{Fix::SessionState::DISCONNECTED},
                serializer_{},
                params_{params},
                clock_{},
                seq_provider_{},
                msg_factory_{params_, seq_provider_, clock_},
                logger_{id_, log_core},
                logon_timer_{exec_},
                inbound_timer_{exec_},
                heartbeat_timer_{exec_},
                logout_timer_{exec_},
                validator_{}
                {
        buff_.resize(Framer::start_size());

    }

    Session::~Session() {
        if (conn_) conn_->close();
    }

    void Session::stop_() {
        if (stopped_) return;
        
        logger_.log(
                {Fix::Error::Layer::Fix, 
                Fix::Error::Category::Info, 
                Fix::Error::Severity::NA},
                "Session stopping"
        );

        if (!conn_) {
            stopped_ = true;
            write_inflight_ = false;
            write_q_.clear();
            return;
        }

      
        stopped_ = true;

        // Cancel timers
        logon_timer_.cancel();
        inbound_timer_.cancel();
        heartbeat_timer_.cancel();
        logout_timer_.cancel();
        
        
        // Close connection (this will cause async reads/writes to complete with ec=operation_aborted)
        if (conn_) {
            conn_->close();
        }

        write_inflight_ = false;
        write_q_.clear();
    }

    void Session::stop() {
        boost::asio::dispatch(exec_, [self = shared_from_this()] {
            self->stop_();
        }); 
    }

    void Session::stop_with_logout(const std::string& reason) {
        boost::asio::dispatch(exec_, [self = shared_from_this(), reason] {
            if (self->stopped_) return;
            self->send_logout(reason);
            
            auto handler  = [self](boost::system::error_code ec) {
                if (self->stopped_ || ec == boost::asio::error::operation_aborted) return;
                if (ec) {
                    self->logger_.log(
                        {Fix::Error::Layer::Fix, 
                        Fix::Error::Category::Error, 
                        Fix::Error::Severity::High},
                        "Couldnt start logout timer"
                    );
                    return;
                };
                self->logger_.log(
                    {Fix::Error::Layer::Fix, 
                    Fix::Error::Category::Info, 
                    Fix::Error::Severity::NA},
                    "Logout timer expired, stopping session"
                );
                self->stop();
            };
            self->logout_timer_.expires_after(std::chrono::seconds(logout_response_timeout));
            self->logout_timer_.async_wait(handler);
            self->state_ = SessionState::AWAITING_LOGOUT;
        });
        
    }

    void Session::start_() {
        logger_.log(
            {Fix::Error::Layer::Fix, 
            Fix::Error::Category::Info, 
            Fix::Error::Severity::NA},
            "Session started"
        );

        if (!conn_) {
            logger_.log(
                {Fix::Error::Layer::Fix, 
                Fix::Error::Category::Error, 
                Fix::Error::Severity::High},
                "Cannot start session without a transport connection"
            );
            return;
        }
        
        do_read(); 

        if (role_ == Fix::Role::INITIATOR) {
            if (params_.initiator_reset_on_logon) {
                seq_provider_.reset();
            }
            send_logon(params_.initiator_reset_on_logon);     
            
            logger_.log(
                {Fix::Error::Layer::Fix, 
                Fix::Error::Category::Info, 
                Fix::Error::Severity::NA},
                "Logon sent"
            );

            state_ = Fix::SessionState::LOGON_SENT;

            schedule_logon_timeout_(Fix::SessionState::LOGON_SENT);
        } else {
            state_ = Fix::SessionState::AWAITING_LOGON;
            schedule_logon_timeout_(Fix::SessionState::AWAITING_LOGON);
        }

    }

    void Session::start() {
        boost::asio::dispatch(exec_, [self = shared_from_this()] {
            self->start_();
        }); 
    }

    
    Log::SessionLogger& Session::logger() {
        return logger_;
    }


    void Session::schedule_logon_timeout_(Fix::SessionState expected_state) {
        auto handler  = [self = shared_from_this(), expected_state](boost::system::error_code ec) {

                if (self->stopped_ || ec == boost::asio::error::operation_aborted) return;
                if (ec) {
                    self->logger_.log(
                        {Fix::Error::Layer::Fix, 
                          Fix::Error::Category::Error, 
                        Fix::Error::Severity::High},
                        "Couldnt start logon timer"
                    );
                    return;
                };


                if (self->state_ == expected_state) {
                    self->logger_.log(
                        {Fix::Error::Layer::Fix, 
                        Fix::Error::Category::Error, 
                        Fix::Error::Severity::High},
                        "Logon timer expired without receiving logon"
                    );

                    self->stop();
                }
            };

            logon_timer_.expires_after(std::chrono::seconds(logon_response_timeout));
            logon_timer_.async_wait(handler);
    }


    std::string Session::readable_id() const noexcept {
        return params_.sender_comp_id + "<->" + params_.target_comp_id + " [" + std::to_string(id_.id) + "]";
    }

    void Session::schedule_heartbeat_() {
        heartbeat_timer_.cancel();
        auto self = shared_from_this();
        heartbeat_timer_.expires_after(std::chrono::seconds(params_.heart_beat_int));
        heartbeat_timer_.async_wait(
            [self](const boost::system::error_code& ec) {
                if (self->stopped_ || ec == boost::asio::error::operation_aborted) return;
                if (ec) {
                    self->logger_.log(
                        {Fix::Error::Layer::Fix, 
                        Fix::Error::Category::Error, 
                        Fix::Error::Severity::High},
                        "Couldnt start heartbeat timer"
                    );
                    return;
                }
                self->send_heartbeat();
                self->schedule_heartbeat_();
            }
        );
       
    }

    void Session::schedule_test_request_timeout_() {
        inbound_timer_.cancel();
        auto self = shared_from_this();
        inbound_timer_.expires_after(std::chrono::seconds(params_.heart_beat_int));
        inbound_timer_.async_wait(
            [self](const boost::system::error_code& ec) {
                if (self->stopped_ || ec == boost::asio::error::operation_aborted) return;
                if (ec) {
                    self->logger_.log(
                        {Fix::Error::Layer::Fix, 
                        Fix::Error::Category::Error, 
                        Fix::Error::Severity::High},
                        "Couldnt start inbound timer"
                    );
                    return;
                }
                
                
                
                if (!self->awaiting_test_request_response_) {   
                    self->awaiting_test_request_response_ = true;
                    std::string testReqId = "TESTREQ_" + std::to_string(self->test_req_id_++);
                    self->logger_.log(
                        {Fix::Error::Layer::Fix, 
                        Fix::Error::Category::Warn, 
                        Fix::Error::Severity::Moderate},
                        "Inbound timeout expired, sent Test Request with ID: " + testReqId
                    );
                    self->send_test_request(testReqId);
                    self->schedule_test_request_timeout_();
                } else {
                    self->logger_.log(
                        {Fix::Error::Layer::Fix, 
                        Fix::Error::Category::Error, 
                        Fix::Error::Severity::High},
                        "No response to Test Request, stopping session"
                    );
                    self->stop_with_logout("No response to Test Request");
                    
                }
                    
            }
        );
    }

    void Session::on_transport_down_() {
        logger_.log(
            {Fix::Error::Layer::Transport, 
            Fix::Error::Category::Info, 
            Fix::Error::Severity::NA},
            "Transport connection lost"
        );
        //cancel timers
        logon_timer_.cancel();
        inbound_timer_.cancel();
        heartbeat_timer_.cancel();
        logout_timer_.cancel();

        //change state
        state_ = Fix::SessionState::DISCONNECTED;
        write_inflight_ = false;
        write_q_.clear();
        

    }

    void Session::send_message_(std::string_view msg_wire, bool is_resend) {    
        auto writer = Fix::WireWriter::from_arena(arena_, msg_wire);
        send_bytes_(std::move(writer));
        if (!is_resend) {
            store_.store_outbound_message(msg_wire);
        }
        if (state_ == SessionState::ACTIVE || state_ == SessionState::RECOVERING_RESEND) {
            schedule_heartbeat_();
        }
    }

    void Session::send_bytes_(Fix::WireWriter handle) {
        write_q_.push_back({std::move(handle), 0});
        if (write_inflight_) return;
        write_inflight_ = true;
        do_write(); 

    }

    void Session::do_read() {
        if (stopped_) return;
       
        auto self = shared_from_this();
        auto boost_buff = boost::asio::buffer(buff_, buff_.size());
        auto handler = boost::asio::bind_executor(exec_,
            [this, self](boost::system::error_code ec, std::size_t n) {

                if (stopped_ || ec == boost::asio::error::operation_aborted) return;

                if (ec) {
                    std::string err_msg = "Read error: " + ec.message();
                    
                    if (Error::classify_readwrite_error(ec) == Fix::Error::RetryClass::Transient) {
                        logger_.log(
                            {Fix::Error::Layer::Transport, 
                            Fix::Error::Category::Warn, 
                            Fix::Error::Severity::Moderate},
                            err_msg
                        );
                        do_read();
                        return;
                    }


                    
                    logger_.log(
                        {Fix::Error::Layer::Transport, 
                        Fix::Error::Category::Error, 
                        Fix::Error::Severity::High},
                        err_msg
                    );
                    conn_->close();
                    return;
                }
                auto sv = std::string_view{buff_.data(), n};
                framer_.append(sv);
                // need to run some timer here
                while (framer_.has_message()) {
                    auto msg = framer_.get_message();
                    process_wire_message_(msg);
                    framer_.consume_message();
                }
                do_read();
            }
        );

    
        conn_->async_read_some(boost_buff, handler);

        
    }

    void Session::do_write() {
        if (write_q_.empty()) {write_inflight_ =  false; return;}
        if (stopped_) return;

        auto& front = write_q_.front();
        auto* base = front.data.data() + front.sent;
        auto left = front.data.size() - front.sent;

        auto self = shared_from_this();
        auto buffer = boost::asio::const_buffer(base, left);
        conn_->async_write_some(
            buffer,
            boost::asio::bind_executor(exec_, 
                [this, self] (boost::system::error_code ec, std::size_t n) {

                    if (stopped_ || ec == boost::asio::error::operation_aborted) return;

                    if (ec) {
                        std::string err_msg = "Write error: " + ec.message();
                        if (Error::classify_readwrite_error(ec) == Fix::Error::RetryClass::Transient) {
                            logger_.log(
                                {Fix::Error::Layer::Transport, 
                                Fix::Error::Category::Warn, 
                                Fix::Error::Severity::Moderate},
                                err_msg
                            );
                            do_write();
                            return;
                        }

                        logger_.log(
                            {Fix::Error::Layer::Transport, 
                            Fix::Error::Category::Error, 
                            Fix::Error::Severity::High},
                            err_msg
                        );
                        
                        conn_->close();
                        write_q_.clear();
                        write_inflight_ = false;
                        return;
                    }

                    
                    
                    auto& f = write_q_.front();
                    f.sent += n;
                    if (f.sent >= f.data.size()) write_q_.pop_front(); 
                
                    do_write();
                }
            
            )
        );
    }

    void Session::set_connection(std::shared_ptr<Fix::IConnection> conn) {
        boost::asio::dispatch(exec_, [self = shared_from_this(), conn = std::move(conn)]() mutable {
            self->conn_ = std::move(conn);
        });
    }

    void Session::drain_recovery_cache_() {
        if (recovery_cache_.empty() || state_ != Fix::SessionState::RECOVERING_RESEND) return;

        auto expected = seq_provider_.next_in();
        while (recovery_cache_.contains(expected)) {

            auto raw_msg = recovery_cache_.get(expected);
            parser_.parse(raw_msg, parser_ctx_.out_msg, parser_ctx_.out_errs);
            if (parser_ctx_.out_errs.empty()) {
                auto msg = parser_ctx_.out_msg;
                dispatch(msg, raw_msg);
            } else {
                // Parse error during recovery, must logout
                // we cant wait for a logout since we are already out of sync
                send_logout("Parse error during recovery");
                stop();
                return;
            }
            recovery_cache_.consume(expected);
            expected = seq_provider_.next_in();
        }

        if (recovery_cache_.empty()) {
            state_ = Fix::SessionState::ACTIVE;
        }
    }

    void Session::process_wire_message_(std::string_view msg_wire) {
        parser_.parse(msg_wire, parser_ctx_.out_msg, parser_ctx_.out_errs);

        if (parser_ctx_.out_errs.empty()) {
            auto msg = parser_ctx_.out_msg;
            dispatch(msg, msg_wire);
        } else {
            stop_with_logout("Parse error");
        } 

        drain_recovery_cache_();
    }


    void Session::dispatch(const GenericMessage<GenericFieldView>& message, std::string_view raw_msg)  {
        Fix::ValidMessage msg = Fix::make_valid_message(message);

        auto results = validator_.validate_message(msg, params_);

        if (results.severity == Error::Severity::Fatal) {
            // Fatal error, must logout
            stop_with_logout("Fatal validation error");
            return;
        }
        
        if (!results.errors.empty()) {
            send_reject(msg.header_cache_.msg_seq_num, static_cast<uint32_t>(results.errors[0].code), results.errors[0].tag, std::string_view(results.errors[0].info));
            return;
        }

        
        // The iterators above should always find the fields since the validator would have caught their absence
        if (msg.header_cache_.slots[static_cast<std::size_t>(CacheSlot::MsgType)] == nullptr || !msg.header_cache_.has_msg_seq_num) {
            logger_.log(
                {Fix::Error::Layer::Fix, 
                Fix::Error::Category::Error, 
                Fix::Error::Severity::High},
                "Validator passed but required fields missing"
            );
            return;
        }
          
        if (state_ == SessionState::ACTIVE || state_ == SessionState::RECOVERING_RESEND) {
            schedule_test_request_timeout_();
        }

        auto& msg_type = *msg.header_cache_.slots[static_cast<std::size_t>(CacheSlot::MsgType)];
        auto seq_num = msg.header_cache_.msg_seq_num;
        bool is_dup = msg.header_cache_.slots[static_cast<std::size_t>(CacheSlot::PossDupFlag)] != nullptr && *msg.header_cache_.slots[static_cast<std::size_t>(CacheSlot::PossDupFlag)] == "Y";


        if (state_ != Fix::SessionState::ACTIVE && state_ != Fix::SessionState::RECOVERING_RESEND) {
            if (is_app_message_type_(msg_type)) {
                stop_with_logout("Cannot process app messages yet");
                return;
            }
        }

        if (seq_num == seq_provider_.next_in()) {
            seq_provider_.update_in(seq_num + 1);
        } else if (seq_num > seq_provider_.next_in() && state_ == Fix::SessionState::RECOVERING_RESEND) {
            // During recovery, cache out-of-order messages
            if (recovery_cache_.in_window(seq_num) == false) {
                stop_with_logout("MsgSeqNum too high during recovery");
                return;
            }
            recovery_cache_.insert(seq_num, raw_msg);
            return;
        } else if (seq_num > seq_provider_.next_in()) {
            // Future seq num, need to resend
            auto expected = seq_provider_.next_in();
            send_resend_request(expected, seq_num - 1);
            state_ = Fix::SessionState::RECOVERING_RESEND;

            recovery_cache_.start(expected);        // base offset = expected
            recovery_cache_.insert(seq_num, raw_msg);
            return;

        } else if (seq_num < seq_provider_.next_in() && is_dup) {
            // drop duplicate
            return;
        } else {
            // old seq num, not marked as duplicate
            stop_with_logout("MsgSeqNum too low");
            return;
        }

        
       

        
     
        

        if (msg_type == "A") {handle_logon(msg);}
        else if (msg_type == "5") {handle_logout(msg);}
        else if (msg_type == "0") {handle_heartbeat(msg);}
        else if (msg_type == "1") {handle_test_request(msg);}
        else if (msg_type == "2") {handle_resend_request(msg);}
        else if (msg_type == "4") {handle_sequence_reset(msg);}
        else {}

       

    }

    Fix::SessionID Session::get_session_id() const noexcept {
        return id_;
    }

    void Session::send_logon(bool reset_seq_nums) {
        auto wire = msg_factory_.logon(params_.heart_beat_int, reset_seq_nums);
        send_message_(wire);
    }

    void Session::send_reject(std::size_t ref_seq_num, uint32_t reason, Tag tag, std::string_view text) {
        auto wire = msg_factory_.reject(ref_seq_num, reason, tag, text);
        send_message_(wire);
    }

    void Session::send_logout(const std::string& reason) {
        auto wire = msg_factory_.logout(reason);
        send_message_(wire);
    }

    void Session::send_heartbeat(const std::string_view testReqId) {
        auto wire = msg_factory_.heart_beat(testReqId);
        send_message_(wire);
    }

    void Session::send_test_request(const std::string& testReqId) {
        auto wire = msg_factory_.test_request(testReqId);
        send_message_(wire);
    }

    void Session::send_resend_request(std::size_t beginSeqNo, std::size_t endSeqNo) {
        auto wire = msg_factory_.resend_request(beginSeqNo, endSeqNo);
        send_message_(wire);
    }

    void Session::send_sequence_reset(std::size_t newSeqNo, bool gapfill) {
        auto wire = msg_factory_.sequence_reset(newSeqNo, gapfill);
        send_message_(wire);
    }

    void Session::handle_logon(const Fix::ValidMessage& message) {
        logger_.log(
            {Fix::Error::Layer::Fix, 
            Fix::Error::Category::Info, 
            Fix::Error::Severity::NA},
            "Logon received"
        );

        auto heart_bt_int_it = std::find_if(
                message.message_.begin(),
                message.message_.end(),
                [](const GenericFieldView& field) {
                    return field.tag == 108; // HeartBtInt
                }
            );
        assert(heart_bt_int_it != message.message_.end());
        validate_heartbeat_int(heart_bt_int_it->value, role_ == Fix::Role::INITIATOR);

        bool reset_seq_nums = false;
        auto reset_it = std::find_if(
            message.message_.begin(),
            message.message_.end(),
            [](const GenericFieldView& field) {
                return field.tag == 141; // ResetSeqNumFlag
            }
        );
        if (reset_it != message.message_.end() && reset_it->value == "Y") {
            reset_seq_nums = true;
        }



        if (role_ == Fix::Role::ACCEPTOR) {
            state_ = Fix::SessionState::LOGON_RECEIVED; // will move to ACTIVE after sending logon response
            if (reset_seq_nums) {
                if (params_.acceptor_reset_on_logon) {
                    seq_provider_.acceptor_reset();
                } else {
                    stop_with_logout("ResetSeqNumFlag=Y not allowed by configuration");
                    return;
                }
            }
            
            send_logon(reset_seq_nums);
            logger_.log(
                {Fix::Error::Layer::Fix, 
                Fix::Error::Category::Info, 
                Fix::Error::Severity::NA},
                "Logon response sent"
            );   
        }  else {
            if (reset_seq_nums) {
                if (!params_.initiator_reset_on_logon) {
                    stop_with_logout("ResetSeqNumFlag=Y not allowed by configuration");
                    return;
                }
            }
        } 
        logon_timer_.cancel(); // cancels automatically on session becoming active, but safe to call here
        state_ = Fix::SessionState::ACTIVE;
        schedule_heartbeat_();  
        schedule_test_request_timeout_();
    }

    void Session::handle_logout(const Fix::ValidMessage& message) {
        logger_.log(
            {Fix::Error::Layer::Fix, 
            Fix::Error::Category::Info, 
            Fix::Error::Severity::NA},
            "Logout received"
        );
        if (state_ == SessionState::AWAITING_LOGOUT) {
            logout_timer_.cancel();
            stop();
            return;
        } else {
            // we reply with logout and stop
            send_logout("Logout acknowledged");
            stop();
            return;
        }
        
    }

    void Session::handle_heartbeat(const Fix::ValidMessage& message) {
        awaiting_test_request_response_ = false;
    }
    void Session::handle_test_request(const Fix::ValidMessage& message) {
        auto test_req_id_it = std::find_if(
            message.message_.begin(),
            message.message_.end(),
            [](const GenericFieldView& field) {
                return field.tag == 112; // TestReqID
            }
        );
        assert(test_req_id_it != message.message_.end());
        send_heartbeat(test_req_id_it->value);
    }
    void Session::handle_resend_request(const Fix::ValidMessage& msg) {
        auto start_seq_num_it = std::find_if(
            msg.message_.begin(),
            msg.message_.end(),
            [](const GenericFieldView& field) {
                return field.tag == 7; // BeginSeqNo
            }
        );
        auto end_seq_num_it = std::find_if(
            msg.message_.begin(),
            msg.message_.end(),
            [](const GenericFieldView& field) {
                return field.tag == 16; // EndSeqNo
            }
        );
        assert(start_seq_num_it != msg.message_.end());
        assert(end_seq_num_it != msg.message_.end());

        // These fields are required and should have been validated already
        std::uint32_t begin_seq_no = 0;
        std::uint32_t end_seq_no = 0;
        std::from_chars(
            start_seq_num_it->value.data(),
            start_seq_num_it->value.data() + start_seq_num_it->value.size(),
            begin_seq_no
        );
        std::from_chars(
            end_seq_num_it->value.data(),
            end_seq_num_it->value.data() + end_seq_num_it->value.size(),
            end_seq_no
        );
        auto stream = store_.get_resend_stream(begin_seq_no, end_seq_no);
        for (; stream.has_next(); ) {
            auto action = stream.next();
            if (action.gap_fill) {
                send_sequence_reset(action.end_seq_no + 1, true);
            } else {
                auto& msg_index = store_.get_message_index(action.begin_seq_no);
                auto new_wire = msg_factory_.regenerate_message(store_.get_message_wire(msg_index), msg_index);
                send_message_(new_wire, true);
            }
        }

    }
    void Session::handle_sequence_reset(const Fix::ValidMessage& msg) {
        auto new_seq_no_it = std::find_if(
            msg.message_.begin(),
            msg.message_.end(),
            [](const GenericFieldView& field) {
                return field.tag == 36; // NewSeqNo
            }
        );
        auto gap_fill_it = std::find_if(
            msg.message_.begin(),
            msg.message_.end(),
            [](const GenericFieldView& field) {
                return field.tag == 123; // GapFillFlag
            }
        );
        assert(new_seq_no_it != msg.message_.end());
        bool gap_fill = false;
        if (gap_fill_it != msg.message_.end() && gap_fill_it->value == "Y") {
            gap_fill = true;
        }
        SeqNum new_seq_no = 0;
        std::from_chars(
            new_seq_no_it->value.data(),
            new_seq_no_it->value.data() + new_seq_no_it->value.size(),
            new_seq_no
        );
        


        if (new_seq_no < seq_provider_.next_in() && !gap_fill) {
            stop_with_logout("NewSeqNo less than expected");
            return;
        }

        seq_provider_.update_in(new_seq_no);
    }


    void Session::validate_heartbeat_int(std::string_view incoming_value, bool is_initiator) {
        uint32_t incoming_hb_int = 0;
        std::from_chars(
            incoming_value.data(),
            incoming_value.data() + incoming_value.size(),
            incoming_hb_int
        );
        if (is_initiator && incoming_hb_int != params_.heart_beat_int) {
            stop_with_logout("HeartBtInt does not match configured value");
            return;
        }
        if (incoming_hb_int < 10 || incoming_hb_int > 120) {
            stop_with_logout("HeartBtInt out of range");
            return;
        }
        params_.heart_beat_int = incoming_hb_int;
    }

    bool Session::is_app_message_type_(std::string_view msg_type) const noexcept {
        return !(msg_type == "0" || msg_type == "1" || msg_type == "2" || msg_type == "3"|| msg_type == "4" || msg_type == "5" || msg_type == "A");
    }

}