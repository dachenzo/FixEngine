#pragma once 
#include <vector>
#include <string_view>
#include <optional>
#include <memory>
#include <fix/IConnection.hpp>
#include <fix/ITimer.hpp>
#include <fix/MessageStore.hpp>
#include <fix/Application.hpp>
#include <fix/Session.hpp>
#include <fix/SessionPool.hpp>
#include <fix/definitions.hpp>

namespace Fix {
    struct SessionManager {

        SessionManager(Fix::Application& app, 
        Fix::IConnectionFactory& connFactory, Fix::ITimerFactory& timerFactory);


        void create_session(Fix::SessionCreationConfig& cnfg);

        bool remove_session(const Fix::SessionID& id);

        std::shared_ptr<Fix::Session> findSession(const Fix::SessionID& id);

        std::vector<Fix::SessionID> listSessions() const;

        void create_all(std::vector<Fix::SessionCreationConfig>& configs);

        void start_all();

        void stop_all();

        std::size_t sessionCount() noexcept;
        
        private:
        Fix::SessionPool session_pool_;
        Fix::Application& app_;
        Fix::MessageStoreFactory store_factory_;
        Fix::IConnectionFactory& connFactory_; 
        Fix::ITimerFactory& timerFactory_;
        

        
        

    };
}