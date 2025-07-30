#include <boost/asio.hpp>
#include <chrono>
#include <memory>
#include "include/fix/ITimer.hpp"



namespace Fix {
    AsioTimerFactory::AsioTimerFactory(boost::asio::io_context& io): io_{io} {}

    std::unique_ptr<ITimer> AsioTimerFactory::create_timer() {
        return std::make_unique<AsioTimer>(io_);
    }

    AsioTimer::AsioTimer(boost::asio::io_context& io): timer_{io} {}

    void AsioTimer::start(std::chrono::milliseconds duration, Fix::ITimer::Handler handler) {
        timer_.expires_after(duration);
        timer_.async_wait(
            [this, handler] (const boost::system::error_code& ec) {
            handler(ec);
        });
    };

    void AsioTimer::cancel() noexcept {
        boost::system::error_code ec;
        timer_.cancel(ec);
    }

}