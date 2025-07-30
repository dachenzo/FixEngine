#include <boost/asio.hpp>
#include <iostream>
#include <chrono>

void sayHelo(const boost::system::error_code& ec) {

    if (ec == boost::asio::error::operation_aborted) {
        std::cout << "Timer Canceled" << '\n';
    }
    std::cout << "Hello" << '\n';
}


int main() {
    boost::asio::io_context io;

    

    boost::asio::steady_timer timer(io);
    std::chrono::milliseconds duration{5000};
    timer.expires_after(duration);
    timer.async_wait(sayHelo);
    boost::system::error_code ec;
    timer.cancel(ec);


    io.run();
}