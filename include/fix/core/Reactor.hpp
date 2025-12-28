#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <functional>
#include <vector>
#include <thread>


namespace Fix {
    struct Reactor {
        
        Reactor();

        ~Reactor();

        boost::asio::io_context& context() noexcept;

        void run(std::size_t n = std::thread::hardware_concurrency());

        void run_single();

        void stop() noexcept;

        void wait();


        private:
        boost::asio::io_context io_context_;
        boost::asio::executor_work_guard<
            boost::asio::io_context::executor_type
        > work_guard_;
        std::vector<std::jthread> workers_;

    };
}