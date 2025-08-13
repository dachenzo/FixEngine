#include <vector>
#include <fix/Reactor.hpp>
#include <fix/definitions.hpp>
#include <fix/SessionManager.hpp>
#include <fix/IConnection.hpp>
#include <fix/ITimer.hpp>
#include <fix/Application.hpp>

/// @brief  Tests that packets are sent between both Initiator and Acceptor configurations and that Fix::Parser is able to emmit a valid message when used
/// @return int
int main() {
    

    Fix::ConnectionConfig client_conn_config {
        "127.0.0.1",
        5001,
        Fix::Role::ACCEPTOR
    };


    Fix::ConnectionConfig server_conn_config {
        "127.0.0.1",
        5001,
        Fix::Role::INITIATOR
    };

    Fix::SessionCreationConfig client_config{
        client_conn_config.role,
        client_conn_config
    };
    
    Fix::SessionCreationConfig server_config {
        server_conn_config.role,
        server_conn_config
    };

    //Configs will come from some configuration file
    std::vector<Fix::SessionCreationConfig> configs{
        client_config, server_config
    };

    Fix::Reactor reactor{};
    Fix::Application app{};
    Fix::AsioConnectionFactory conn_factory{reactor.context()};
    Fix::AsioTimerFactory timer_factory{reactor.context()};
    Fix::SessionManager session_manager{
        app,
        conn_factory,
        timer_factory
    };

    session_manager.create_all(configs);

    reactor.run();
    reactor.stop();

    // session_manager.create_all(configs);
    // session_manager.start_all();




}

