#pragma once
#include <vector>
#include <memory>
#include <span>
#include <fix/core/Session.hpp>
#include <fix/core/definitions.hpp>
#include <fix/core/IConnection.hpp>
#include <fix/core/MessageStore.hpp>
#include <fix/core/Message.hpp>
#include <fix/core/Application.hpp>
#include <fix/core/ITimer.hpp>
#include <fix/core/Codec.hpp>
#include <fix/core/Parser.hpp>
#include <fix/log/LogCore.hpp>


namespace Fix {

    struct SessionPool {

        SessionPool();

        std::size_t size() noexcept;

        std::shared_ptr<Session> emplace_session(
                Fix::Role role,
                Fix::Application& app,
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