#include <boost/asio.hpp>
#include <string_view>
#include <string>
#include <iostream>
#include <charconv>
#include <optional>
#include <fix/Session.hpp>
#include <fix/Parser.hpp>
#include <fix/utils.hpp>

namespace Fix {
    constexpr const int wire_pre_alloc = 200;
    constexpr const int msg_type_key = 35;
    constexpr const int msg_seq_num_key = 34;
    Session::Session(
                Fix::SessionID id,
                Fix::Role role,
                Fix::Application& app,
                Fix::ITimerFactory& timers,
                Fix::SessionParameters params
            ):  
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
                seq_provider_{store_},
                msg_factory_{params_, seq_provider_, clock_}
                {
        buff_.resize(8192);

    }

    void Session::stop() {
        std::cout << "Session_stopped" <<'\n';
    }

    Session::~Session() {
        conn_->close();
    }



    void Session::start() {
        auto role_str = role_ == Fix::Role::INITIATOR ? "Iniator" : "Acceptor";
        std::cout << role_str << "Started\n";
        do_read(); 

        if (role_ == Fix::Role::INITIATOR) {
            send_logon();
            state_ = Fix::SessionState::LOGON_SENT;
        } else {
            state_ = Fix::SessionState::AWAITING_LOGON;
        }

    }

    void print_escaped(const std::string& s) {
        for (unsigned char c : s) {
            if (std::isprint(c)) {
                std::cout << c;
            } else {
                std::cout << "\\x"
                        << std::hex << std::setw(2) << std::setfill('0')
                        << static_cast<int>(c)
                        << std::dec; // reset back to decimal
            }
        }
        std::cout << "\n";
    }

    void Session::send_message_(Fix::Message& msg) {
        std::string wire{};
        wire.reserve(wire_pre_alloc);
        std::size_t wire_len = serializer_.serialize(msg, wire);
        int seq = store_.get_next_sender_seq();
        store_.store_outbound(seq, msg);
        send_bytes_(std::move(wire));
    }

    void Session::send_bytes_(std::string msg_wire) {
        write_q_.push_back({std::move(msg_wire), 0});
        if (write_inflight_) return;
        write_inflight_ = true;
        do_write();

    }

    void Session::do_read() {
       
        auto self = shared_from_this();
        auto boost_buff = boost::asio::buffer(buff_, buff_.size());
        auto handler = [this, self](boost::system::error_code ec, std::size_t n) {
                if (ec) {
                    // tear down on error
                    std::cout << "Error reading\n";
                    conn_->close();
                    return;
                }
                auto sv = std::string_view{buff_.data(), n};
                std::cout << sv.size() << '\n';
                std::cout << sv << '\n';
                auto msg = parser_.parse(sv);

                if (msg.has_value()) {
                    std::cout << "got a message\n";
                    dispatch(msg.value());
                    std::cout << "Called Dispatch\n";
                    
                } 

                do_read();
        };

    
        conn_->async_read_some(boost_buff, handler);

        
    }

    void Session::do_write() {
        if (write_q_.empty()) {write_inflight_ =  false; return;}

        auto& front = write_q_.front();
        auto* base = front.data.data() + front.sent;
        auto left = front.data.size() - front.sent;

        auto self = shared_from_this();
        auto buffer = boost::asio::const_buffer(base, left);
        conn_->async_write_some(
            buffer,
            [this, self] (boost::system::error_code ec, std::size_t n) {
                if (ec) {
                    std::cout << "Error writing\n";
                    conn_->close();
                    write_q_.clear();
                    write_inflight_ = false;
                    return;
                }
                
                auto& f = write_q_.front();
                f.sent += n;
                std::cout << "Wrote\n";
                if (f.sent >= f.data.size()) write_q_.pop_front(); std::cout << "Written\n";
               
                do_write();
            }
        );
    }

    void Session::set_connection(std::shared_ptr<Fix::IConnection> conn) {
        conn_ = conn;
    }


    void Session::dispatch(Fix::Message& msg) {
       
        auto seqnum_sv = msg.get(msg_seq_num_key);
        if (!seqnum_sv.has_value()) {throw std::runtime_error("Every message should have a sequence number");}
        int seqnum;
        auto [ptr, ec] = std::from_chars(seqnum_sv.value().data(), seqnum_sv.value().data() + seqnum_sv.value().size(), seqnum);
        if (!(ec == std::errc())) {throw std::runtime_error("sequence number string couldnt be converted to integer");}

        

        std::cout << "Here0";

        auto op_type = msg.get(35);
    
        
        if (!op_type.has_value()) {throw std::runtime_error("Message Must have a type");}
        std::string_view type = op_type.value();
     
        

        if (type == "A") {handle_logon(msg);}
        else if (type == "5") {handle_logout(msg);}
        else if (type == "0") {handle_heartbeat(msg);}
        else if (type == "1") {handle_test_request(msg);}
        else if (type == "2") {handle_resend_request(msg);}
        else if (type == "4") {handle_sequence_reset(msg);}
        else {}

        store_.store_inbound(seqnum, msg);

    }

    Fix::SessionID& Session::get_session_id() noexcept {
        return id_;
    }

    void Session::send_logon() {
        std::cout << "Got to send logon\n";
        auto msg = msg_factory_.logon(0, true);
        std::cout << "Message created\n";
        send_message_(msg);
        std::cout << "Message sent\n";

    }

    void Session::handle_logon(const Fix::Message&) {
        std::cout << " Logon received";
    }
    void Session::handle_logout(const Fix::Message&) {}
    void Session::handle_heartbeat(const Fix::Message&) {}
    void Session::handle_test_request(const Fix::Message&) {}
    void Session::handle_resend_request(const Fix::Message&) {}
    void Session::handle_sequence_reset(const Fix::Message&) {}

}