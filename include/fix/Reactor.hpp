#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <functional>


namespace Fix {
    struct Reactor {
        
        Reactor();

        ~Reactor();

        boost::asio::io_context& context() noexcept;

        void run();

        void stop();


        private:
        boost::asio::io_context ioCtx_;
        boost::asio::executor_work_guard<
            boost::asio::io_context::executor_type
        > work_guard_;

    };
}