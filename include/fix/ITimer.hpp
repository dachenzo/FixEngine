#pragma once 

#include <chrono>
#include <functional>
#include <memory>
#include <system_error>
#include <boost/asio.hpp>


namespace Fix {

    struct ITimer {
        using Handler = std::function<void(const std::error_code)>;
            
    
        virtual ~ITimer() = default;

       

        virtual void start(std::chrono::milliseconds duration, Handler& handler) = 0;

        virtual void cancel() noexcept = 0;
    };

    struct ITimerFactory {
        virtual ~ITimerFactory() = default;

        virtual std::unique_ptr<ITimer> create_timer() = 0;
    };

    
    struct AsioTimer: ITimer {
     
        AsioTimer(boost::asio::io_context& io);

        void start(std::chrono::milliseconds duration, Handler& handler);

        void cancel() noexcept;

        private:
        boost::asio::steady_timer timer_;

        
    };

    struct AsioTimerFactory: ITimerFactory {
        AsioTimerFactory(boost::asio::io_context& io);

        std::unique_ptr<ITimer> create_timer();

        private:
        boost::asio::io_context& io_;
    };




}

