#pragma once
#include <vector>
#include <memory>
#include <span>
#include <fix/core/definitions.hpp>
#include <fix/core/ApplicationEvents.hpp>
#include <fix/core/ITimer.hpp>
#include <fix/log/LogCore.hpp>
#include <boost/asio/io_context.hpp>


namespace Fix {

    struct Session;

    struct SessionPool {

        SessionPool();

        std::size_t size() noexcept;

        std::shared_ptr<Fix::Session> emplace_session(
                Fix::Role role,
                Fix::AppSink&& app_sink,
                Fix::ITimerFactory& timers,
                Fix::SessionParameters params,
                Fix::Log::LogCore& log_core,
                boost::asio::io_context& io_context,
                ReconnectCallback& reconnect_callback
            );

        bool remove(Fix::SessionID id);

        std::shared_ptr<Fix::Session> get(Fix::SessionID id);

        std::span<std::shared_ptr<Fix::Session>> get_all_at_start();

        std::vector<std::shared_ptr<Fix::Session>> get_all();


        private:
        std::vector<std::shared_ptr<Fix::Session>> sessions_;
        std::vector<std::size_t> free_indices_;
        std::size_t next_idx = 0;

        std::size_t generate_session_Id_() noexcept;
    };

}