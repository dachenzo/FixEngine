#include <boost/asio.hpp>
#include <string_view>
#include <iostream>
#include <optional>
#include <fix/Session.hpp>
#include <fix/Parser.hpp>

namespace Fix {


    Session::Session(
                Fix::SessionID id,
                Fix::Role role,
                std::unique_ptr<IConnection> conn,
                Fix::Application& app,
                Fix::ITimerFactory& timers):  
                conn_{std::move(conn)}, 
                parser_{},
                app_{app},
                timers_{timers},
                id_{id},
                role_{role},
                store_{}
                {
        buff_.reserve(8192);

    }

    void Session::stop() {
        std::cout << "Session_stopped" <<'\n';
    }

    Session::~Session() {
        conn_->close();
    }

    void Session::start() {
        do_read();
    }

    void Session::do_read() {
        auto self = shared_from_this();
        auto boost_buff = boost::asio::buffer(buff_, buff_.size());
        auto handler = [this, self](boost::system::error_code ec, std::size_t n) {
                if (ec) {
                    // tear down on error
                    conn_->close();
                    return;
                }
                auto sv = std::string_view{buff_.data(), n};
                auto msg = parser_.parse(sv);

                if (msg.has_value()) {
                    dispatch(msg.value());
                }

                do_read();
        };

    
        conn_->async_read_some(boost_buff, handler);

        
    }


    void Session::dispatch(const Fix::Message& msg) {
        for (auto& field: msg.get_fields()) {
            std::cout << field.tag << '=' << field.value << '\n';
        }
    }

    Fix::SessionID& Session::get_session_id() noexcept {
        return id_;
    }

}