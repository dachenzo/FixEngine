#include <boost/asio.hpp>
#include <fix/Reactor.hpp>
#include <iostream>


namespace Fix {
    Reactor::Reactor(): io_context_(), work_guard_(boost::asio::make_work_guard(io_context_)) {

    }

    Reactor::~Reactor() {
        stop();
    }

    boost::asio::io_context& Reactor::context() noexcept {
        return io_context_;
    }

    void Reactor::run_single() {
        io_context_.run();
    }


    void Reactor::run(std::size_t n_threads) {
        if (!workers_.empty()) return;
        n_threads = std::max(static_cast<std::size_t>(1), n_threads);
        workers_.reserve(n_threads);

        for (std::size_t i = 0; i < n_threads; i++) {
            workers_.emplace_back(
                [this](){
                    io_context_.run();
                }
            );
        }
    }

    void Reactor::stop() noexcept {
        work_guard_.reset();
        io_context_.stop();
    }

    void Reactor::wait() {
        // Destroy all jthreads → they join their threads
        workers_.clear();
    }
}