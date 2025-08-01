#include <boost/asio.hpp>
#include <fix/Reactor.hpp>


namespace Fix {
    Reactor::Reactor(): io_context_(), work_guard_(boost::asio::make_work_guard(io_context_)) {

    }

    Reactor::~Reactor() {
        stop();
    }

    boost::asio::io_context& Reactor::context() noexcept {
        return io_context_;
    }

    void Reactor::run() {
        io_context_.run();
    }

    void Reactor::stop(){
        work_guard_.reset();
        io_context_.stop();
    }
}