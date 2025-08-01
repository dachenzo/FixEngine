#pragma once
#include <vector>
#include <memory>
#include <functional>
#include <span>
#include <optional>
#include <fix/Session.hpp>
#include <fix/definitions.hpp>
#include <fix/IConnection.hpp>
#include <fix/MessageStore.hpp>
#include <fix/Message.hpp>
#include <fix/Application.hpp>
#include <fix/ITimer.hpp>
#include <fix/Codec.hpp>
#include <fix/Parser.hpp>


namespace Fix {

    struct SessionPool {

        SessionPool();

        std::size_t size() noexcept;

        void emplace_session(
                Fix::Role role,
                std::unique_ptr<IConnection> conn,
                Fix::Application& app,
                Fix::ITimerFactory& timers);

        bool remove(Fix::SessionID id);

        std::shared_ptr<Fix::Session> get(Fix::SessionID id);

        std::span<std::shared_ptr<Fix::Session>> get_all_at_start();

        std::vector<std::shared_ptr<Fix::Session>> get_all();


        private:
        std::vector<std::shared_ptr<Fix::Session>> sessions_;
        std::vector<std::size_t> free_indices_;
        std::size_t next_idx;

        std::size_t generate_session_Id_() noexcept;
    };

}