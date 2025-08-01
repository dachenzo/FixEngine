#include <vector>
#include <fix/Reactor.hpp>
#include <fix/definitions.hpp>
#include <fix/SessionManager.hpp>
#include <fix/IConnection.hpp>
#include <fix/ITimer.hpp>
#include <fix/Application.hpp>


int main() {

    //Configs will come from some configuration file
    std::vector<Fix::SessionCreationConfig> configs;

    Fix::Reactor reactor{};
    Fix::Application app{};
    Fix::AsioConnectionFactory conn_factory{reactor.context()};
    Fix::AsioTimerFactory timer_factory{reactor.context()};
    Fix::SessionManager session_manager{
        app,
        conn_factory,
        timer_factory
    };

    reactor.run();
    reactor.stop();

    // session_manager.create_all(configs);
    // session_manager.start_all();




}

