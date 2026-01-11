#include <boost/asio.hpp>
#include <string_view>
#include <string>
#include <iostream>
#include <charconv>
#include <optional>
#include <fix/log/LogEntry.hpp>
#include <fix/error/Network.hpp>
#include <fix/core/Session.hpp>
#include <fix/core/Parser.hpp>
#include <fix/core/utils.hpp>

namespace Fix {

    constexpr const int msg_type_key = 35;
    constexpr const int msg_seq_num_key = 34;
    Session::Session(
                Fix::SessionID id,
                Fix::Role role,
                Fix::Application& app,
                Fix::ITimerFactory& timers,
                Fix::SessionParameters params,
                Fix::Log::LogCore& log_core
            ):  
                arena_{},
                parser_{},
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
                validator_{}
                {
        buff_.resize(8192);

    }

    void Session::stop() {
        if (stopped_) return;
        auto self = shared_from_this();
        self->logger_.log(
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

        boost::asio::dispatch(exec_, [self] {
            if (self->stopped_) return;
            self->stopped_ = true;

            // Cancel timers
            self->logon_timer_.cancel();
            self->inbound_timer_.cancel();
            self->heartbeat_timer_.cancel();
            
            
            // Close connection (this will cause async reads/writes to complete with ec=operation_aborted)
            if (self->conn_) {
                self->conn_->close();
            }

            self->write_inflight_ = false;
            self->write_q_.clear();
    });
        
    }

    Session::~Session() {
        if (conn_) conn_->close();
    }

    Log::SessionLogger& Session::logger() {
        return logger_;
    }


    void Session::schedule_logon_timeout_(Fix::SessionState expected_state) {
        auto handler  = [self = shared_from_this(), expected_state](std::error_code ec) {
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



    void Session::start() {
        logger_.log(
            {Fix::Error::Layer::Fix, 
            Fix::Error::Category::Info, 
            Fix::Error::Severity::NA},
            "Session started"
        );
        
        do_read(); 

        if (role_ == Fix::Role::INITIATOR) {
            send_logon();     
            
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

    std::string Session::readable_id() const noexcept {
        return params_.sender_comp_id + "<->" + params_.target_comp_id + " [" + std::to_string(id_.id) + "]";
    }

    void Session::send_message_(std::string_view msg_wire) {    
        auto writer = Fix::WireWriter::from_arena(arena_, msg_wire);
        send_bytes_(std::move(writer));
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

                // need to run some timer here
                auto parse_res = parser_.parse(sv);

                if (parse_res.errs.empty() && parse_res.message.has_value()) {
                    auto msg = parse_res.message.value();
                    dispatch(msg);
                } else if (!parse_res.errs.empty()) {
                    send_logout("Parse error");
                    stop();
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
        conn_ = conn;
        exec_ = boost::asio::strand<boost::asio::any_io_executor>(conn_->get_executor());
        logon_timer_ = boost::asio::steady_timer(exec_);
        inbound_timer_ = boost::asio::steady_timer(exec_);
        heartbeat_timer_ = boost::asio::steady_timer(exec_);
    }


    void Session::dispatch(const Message::GenericMessage& message) {
        Fix::ValidMessage msg = Fix::make_valid_message(message);



        auto results = validator_.validate_message(msg, params_);

        if (results.severity == Error::Severity::Fatal) {
            // Fatal error, must logout
            send_logout("Fatal validation error");
            stop();
            return;
        }
        
        if (!results.errors.empty()) {
            // Reject Message
            // send_reject();
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
            // send_reject();
            return;
        }

        auto& msg_type = *msg.header_cache_.slots[static_cast<std::size_t>(CacheSlot::MsgType)];
        auto seq_num = msg.header_cache_.msg_seq_num;
         bool is_dup = msg.header_cache_.slots[static_cast<std::size_t>(CacheSlot::PossDupFlag)] != nullptr &&
                      *msg.header_cache_.slots[static_cast<std::size_t>(CacheSlot::PossDupFlag)] == "Y";

        if (seq_num == seq_provider_.next_in()) {
            seq_provider_.update_in(seq_num + 1);
        } else if (seq_num > seq_provider_.next_in()) {
            // Future seq num, need to resend
            send_resend_request(seq_provider_.next_in(), 0);
            return;
        } else if (seq_num < seq_provider_.next_in() && is_dup) {
            // drop duplicate
            return;
        } else {
            // old seq num, not marked as duplicate
            send_logout("MsgSeqNum too low");
            stop();
            return;
        }

        
       

        
     
        

        if (msg_type == "A") {handle_logon(msg);}
        else if (msg_type == "5") {handle_logout(msg);}
        else if (msg_type == "0") {handle_heartbeat(msg);}
        else if (msg_type == "1") {handle_test_request(msg);}
        else if (msg_type == "2") {handle_resend_request(msg);}
        else if (msg_type == "4") {handle_sequence_reset(msg);}
        else {}

        // store_.store_inbound(seqnum, msg);

    }

    Fix::SessionID Session::get_session_id() const noexcept {
        return id_;
    }

    void Session::send_logon() {
        auto wire = msg_factory_.logon(params_.heart_beat_int, params_.reset_on_logon);
        send_message_(wire);
    }

    void Session::send_reject(std::size_t ref_seq_num, uint32_t reason, std::size_t tag, std::string text) {
        auto wire = msg_factory_.reject(ref_seq_num, reason, tag, text);
        send_message_(wire);
    }

    void Session::send_logout(const std::string& reason) {
        auto wire = msg_factory_.logout(reason);
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

    void Session::handle_logon(const Fix::ValidMessage&) {
        logger_.log(
            {Fix::Error::Layer::Fix, 
            Fix::Error::Category::Info, 
            Fix::Error::Severity::NA},
            "Logon received"
        );

        if (role_ == Fix::Role::ACCEPTOR) {
            state_ = Fix::SessionState::LOGON_RECEIVED; // will move to ACTIVE after sending logon response
        } else {
            state_ = Fix::SessionState::ACTIVE;
        }
    }
    void Session::handle_logout(const Fix::ValidMessage&) {}
    void Session::handle_heartbeat(const Fix::ValidMessage&) {}
    void Session::handle_test_request(const Fix::ValidMessage&) {
        
    }
    void Session::handle_resend_request(const Fix::ValidMessage& msg) {
        auto start_seq_num_it = std::find_if(
            msg.message_.begin(),
            msg.message_.end(),
            [](const Message::GenericField& field) {
                return field.tag == 7; // BeginSeqNo
            }
        );
        auto end_seq_num_it = std::find_if(
            msg.message_.begin(),
            msg.message_.end(),
            [](const Message::GenericField& field) {
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
                send_message_(new_wire);
            }
        }

    }
    void Session::handle_sequence_reset(const Fix::ValidMessage&) {}

}