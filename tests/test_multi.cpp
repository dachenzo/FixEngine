#include <vector>
#include <iostream>
#include <string>

#include <fix/Reactor.hpp>
#include <fix/definitions.hpp>
#include <fix/SessionManager.hpp>
#include <fix/IConnection.hpp>
#include <fix/ITimer.hpp>
#include <fix/Application.hpp>

int main() {
    std::cout << "Multi-session FIX test started\n";

    // --- Connection configs -------------------------------------------------
    // Pair 1: CLT1 <-> SRV1 on 5001
    Fix::ConnectionConfig acc1_conn {
        "127.0.0.1",
        5001,
        Fix::Role::ACCEPTOR,
        /*backlog*/ 16
    };

    Fix::ConnectionConfig init1_conn {
        "127.0.0.1",
        5001,
        Fix::Role::INITIATOR,
        /*backlog*/ 0   // ignored for initiator, but must be set
    };

    // Pair 2: CLT2 <-> SRV2 on 5002
    Fix::ConnectionConfig acc2_conn {
        "127.0.0.1",
        5002,
        Fix::Role::ACCEPTOR,
        /*backlog*/ 16
    };

    Fix::ConnectionConfig init2_conn {
        "127.0.0.1",
        5002,
        Fix::Role::INITIATOR,
        /*backlog*/ 0
    };

    // --- Session parameters (per logical session) ---------------------------
    // You can override defaults per session as needed.

    Fix::SessionParameters srv1_params{};
    srv1_params.sender_comp_id = "SRV1";
    srv1_params.target_comp_id = "CLT1";
    srv1_params.fix_version    = "FIX.4.4";
    srv1_params.heart_beat_int = 30;

    Fix::SessionParameters clt1_params{};
    clt1_params.sender_comp_id = "CLT1";
    clt1_params.target_comp_id = "SRV1";
    clt1_params.fix_version    = "FIX.4.4";
    clt1_params.heart_beat_int = 30;

    Fix::SessionParameters srv2_params{};
    srv2_params.sender_comp_id = "SRV2";
    srv2_params.target_comp_id = "CLT2";
    srv2_params.fix_version    = "FIX.4.4";
    srv2_params.heart_beat_int = 20; // different HB to verify behaviour

    Fix::SessionParameters clt2_params{};
    clt2_params.sender_comp_id = "CLT2";
    clt2_params.target_comp_id = "SRV2";
    clt2_params.fix_version    = "FIX.4.4";
    clt2_params.heart_beat_int = 20;

    // --- Session creation configs ------------------------------------------
    Fix::SessionCreationConfig acc1_session {
        Fix::Role::ACCEPTOR,
        acc1_conn,
        clt1_params
    };

    Fix::SessionCreationConfig init1_session {
        Fix::Role::INITIATOR,
        init1_conn,
        srv1_params
    };

    Fix::SessionCreationConfig acc2_session {
        Fix::Role::ACCEPTOR,
        acc2_conn,
        clt2_params
    };

    Fix::SessionCreationConfig init2_session {
        Fix::Role::INITIATOR,
        init2_conn,
        srv2_params
    };

    std::vector<Fix::SessionCreationConfig> configs {
        acc1_session,
        init1_session,
        acc2_session,
        init2_session
    };

    // --- Core engine objects -----------------------------------------------
    Fix::Reactor reactor{};
    std::cout << "Reactor created\n";

    Fix::Application app{};
    std::cout << "Application created\n";

    Fix::AsioConnectionFactory conn_factory{reactor.context()};
    std::cout << "AsioConnectionFactory created\n";

    Fix::AsioTimerFactory timer_factory{reactor.context()};
    std::cout << "AsioTimerFactory created\n";

    Fix::SessionManager session_manager{
        app,
        conn_factory,
        timer_factory
    };
    std::cout << "SessionManager created\n";

    // Create all sessions (2 acceptors + 2 initiators)
    session_manager.create_all(configs);
    std::cout << "All sessions created\n";

    // --- Run reactor threads -----------------------------------------------
    reactor.run(4); // e.g. 4 worker threads
    std::cout << "Reactor running\n";

    std::cout << "Press ENTER to stop...\n";
    std::string dummy;
    std::getline(std::cin, dummy);

    // --- Graceful shutdown --------------------------------------------------
    session_manager.stop_all();
    reactor.stop();
    reactor.wait();

    std::cout << "Multi-session FIX test finished\n";
    return 0;
}
