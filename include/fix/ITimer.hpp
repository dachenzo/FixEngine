#pragma once 

#include <chrono>
#include <functional>
#include <memory>
#include <system_error>
#include <boost/asio.hpp>


namespace Fix {

    struct ITimerFactory {
        virtual ~ITimerFactory() = default;

        virtual std::unique_ptr<ITimer> create_timer() = 0;
    };

    struct ITimer {
        using Handler = std::function<void(const std::error_code)>;
            
    
        virtual ~ITimer() = default;

       

        virtual void start(std::chrono::milliseconds duration, Handler handler) = 0;

        virtual void cancel() = 0;
    };


    struct AsioTimerFactory: ITimerFactory {
        AsioTimerFactory(boost::asio::io_context& io);

        std::unique_ptr<ITimer> create_timer();

        private:
        boost::asio::io_context& io_;
    };


    struct AsioTimer: ITimer {
     
        AsioTimer::AsioTimer(boost::asio::io_context& io);

        virtual void start(std::chrono::milliseconds duration, Handler handler) = 0;

        void cancel();

        private:
        boost::asio::steady_timer timer_;

        
    };

}

