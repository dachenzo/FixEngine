#include <boost/asio.hpp>
#include <string_view>
#include <optional>
#include "include/fix/Session.hpp"
#include "include/fix/Parser.hpp"

namespace Fix {


    Session::Session(Fix::SessionID id,
                Fix::Role role,
                std::shared_ptr<IConnection> conn,
                Fix::Codec& codec,
                Fix::MessageStore& store,
                Fix::Application& app,
                Fix::ITimerFactory& timers):  conn_{std::move(conn)}, parser_{}{
        buff_.reserve(8192);

    }

    void Session::start() {
        do_read();
    }

    void Session::do_read() {
        auto self = shared_from_this();
        auto boost_buff = boost::asio::buffer(buff_, buff_.size());
        conn_->async_read_some(
            boost_buff,
            [this, self](boost::system::error_code ec, std::size_t n) {
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
            });

        
    }

}