#include <vector>
#include <iostream>
#include <fix/Reactor.hpp>
#include <fix/definitions.hpp>
#include <fix/SessionManager.hpp>
#include <fix/IConnection.hpp>
#include <fix/ITimer.hpp>
#include <fix/Application.hpp>

/// @brief  Tests that packets are sent between both Initiator and Acceptor configurations and that Fix::Parser is able to emmit a valid message when used
/// @return int
int main() {
    std::cout << "Server Test started\n";


    Fix::ConnectionConfig server_conn_config {
        "127.0.0.1",
        5001,
        Fix::Role::INITIATOR,
        1
    };

    
    Fix::SessionCreationConfig server_config {
        server_conn_config.role,
        server_conn_config
    };

    //Configs will come from some configuration file
    std::vector<Fix::SessionCreationConfig> configs{
        server_config
    };

    Fix::Reactor reactor{};
    std::cout << "Reactor Created\n";
    Fix::Application app{};
    std::cout << "App Created\n";
    Fix::AsioConnectionFactory conn_factory{reactor.context()};
    std::cout << "AsioConnFactory Created\n";
    Fix::AsioTimerFactory timer_factory{reactor.context()};
    std::cout << "TimerFactory Created\n";
    Fix::SessionManager session_manager{
        app,
        conn_factory,
        timer_factory
    };
    std::cout << "Session Manager Created\n";
    session_manager.create_all(configs);
    std::cout << "Sessions created\n";

    reactor.run();



}

