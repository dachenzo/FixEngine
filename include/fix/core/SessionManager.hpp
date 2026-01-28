#pragma once 
#include <boost/asio/executor.hpp>
#include <unordered_map>
#include <vector>
#include <memory>
#include <fix/core/IConnection.hpp>
#include <fix/core/ITimer.hpp>
#include <fix/core/MessageStore.hpp>
#include <fix/core/Application.hpp>
#include <fix/core/Session.hpp>
#include <fix/core/SessionPool.hpp>
#include <fix/core/definitions.hpp>
#include <fix/log/LogCore.hpp>

namespace Fix {
    

    struct SessionManager: std::enable_shared_from_this<Fix::SessionManager> {

        static std::string generate_engine_id();

        SessionManager(Fix::Application& app, 
        Fix::IConnectionFactory& connFactory, Fix::ITimerFactory& timerFactory, boost::asio::io_context& io_context);
        SessionManager(const SessionManager& other) = delete;
        SessionManager& operator=(const SessionManager& other) = delete;
        SessionManager(const SessionManager&& other) = delete;
        SessionManager& operator=(const SessionManager&& other) = delete;


        void create_session(const Fix::SessionCreationConfig& cnfg);

        bool remove_session(const Fix::SessionID& id);

        std::shared_ptr<Fix::Session> findSession(const Fix::SessionID& id);

        std::vector<Fix::SessionID> listSessions() const;

        void create_all(std::vector<Fix::SessionCreationConfig>& configs);

        void start_all();

        void stop_all();

        std::size_t sessionCount() noexcept;
        
        private:

        void reconnect_session_impl_(const Fix::SessionID& id);

        Fix::Log::LogCore log_core_;
        boost::asio::strand<boost::asio::any_io_executor> exec_;
        Fix::SessionPool session_pool_;
        std::unordered_map<Fix::SessionID, Fix::SessionCreationConfig> session_configs_;
        Fix::Application& app_;
        Fix::IConnectionFactory& connFactory_; 
        Fix::ITimerFactory& timerFactory_;
        boost::asio::io_context& io_context_;
        

        

        

        
        

    };
}