#include <gtest/gtest.h>
#include <fix/core/SessionManager.hpp>
#include <fix/core/IConnection.hpp>
#include <fix/core/LoopbackTransport.hpp>
#include <fix/core/ApplicationEvents.hpp>
#include <boost/asio.hpp>
#include <thread>
#include <deque>

using namespace Fix;

namespace {
static void pump(boost::asio::io_context& io, int iters = 200) {
    for (int i = 0; i < iters; ++i) {
        if (io.poll() == 0) {
             io.restart();
             break;
        }
        io.restart();
    }
}

static bool contains_fix_field(std::string_view wire, std::string_view needle) {
    return wire.find(needle) != std::string_view::npos;
}

static const std::string SOH = "\x01";
}

class MockConnectionFactory : public IConnectionFactory {
public:
    boost::asio::io_context& io_context;
    
    MockConnectionFactory(boost::asio::io_context& io) : io_context(io) {}

    struct Call {
        std::string type; // "connect" or "listen"
        ConnectionConfig config;
    };
    std::vector<Call> calls;

    std::deque<std::shared_ptr<IConnection>> next_connections;

    void add_connection(std::shared_ptr<IConnection> conn) {
        next_connections.push_back(conn);
    }

    void async_connect(const ConnectionConfig& cfg, ConnectHandler handler) override {
        calls.push_back({"connect", cfg});
        if (!next_connections.empty()) {
             auto conn = next_connections.front();
             next_connections.pop_front();
             boost::asio::post(io_context, [handler, conn]() {
                 handler(boost::system::error_code(), conn);
             });
        }
    }

    void async_listen(const ConnectionConfig& cfg, ConnectHandler handler) override {
        calls.push_back({"listen", cfg});
        if (!next_connections.empty()) {
             auto conn = next_connections.front();
             next_connections.pop_front();
             boost::asio::post(io_context, [handler, conn]() {
                 handler(boost::system::error_code(), conn);
             });
        }
    }
};

TEST(SessionManagerTest, ReconnectsOnDisconnect) {
    boost::asio::io_context io;
    MockConnectionFactory factory(io);
    AppSink sink = [](InBoundAppEvent&&){};
    SessionManager manager(factory, io, std::move(sink));

    SessionCreationConfig config;
    config.role = Role::INITIATOR;
    config.conn_config.role = Role::INITIATOR;
    config.conn_config.ip = "127.0.0.1";
    config.conn_config.port = 9000;
    
    auto [c1, c2] = TestSupport::LoopbackConnection::make_pair(io);
    factory.add_connection(c1);

    manager.create_session(config);
    
    pump(io);

    ASSERT_EQ(factory.calls.size(), 1);
    EXPECT_EQ(factory.calls[0].type, "connect");

    // After connect, Initiator should send Logon
    EXPECT_TRUE(contains_fix_field(c1->peek_write_log(), "35=A")) << "Logon before disconnect";

    auto [c3, c4] = TestSupport::LoopbackConnection::make_pair(io);
    factory.add_connection(c3);

    c1->simulate_disconnect();

    int max_retries = 20;
    while (factory.calls.size() < 2 && max_retries-- > 0) {
        pump(io);
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    ASSERT_EQ(factory.calls.size(), 2);
    EXPECT_EQ(factory.calls[1].type, "connect");

    // After reconnect, Initiator should send Logon again on new connection (c3)
    pump(io);
    EXPECT_TRUE(contains_fix_field(c3->peek_write_log(), "35=A")) << "Logon after reconnect";
}

TEST(SessionManagerTest, LogonHandshake) {
    boost::asio::io_context io;
    MockConnectionFactory factory(io);
    AppSink sink = [](InBoundAppEvent&&){};
    SessionManager manager(factory, io, std::move(sink));

    SessionCreationConfig init_config;
    init_config.role = Role::INITIATOR;
    init_config.conn_config.role = Role::INITIATOR;
    init_config.conn_config.ip = "127.0.0.1";
    init_config.conn_config.port = 9000;
    init_config.params.sender_comp_id = "INIT";
    init_config.params.target_comp_id = "ACC";
    init_config.params.heart_beat_int = 10;

    SessionCreationConfig acc_config;
    acc_config.role = Role::ACCEPTOR;
    acc_config.conn_config.role = Role::ACCEPTOR;
    acc_config.conn_config.ip = "127.0.0.1";
    acc_config.conn_config.port = 9000;
    acc_config.params.sender_comp_id = "ACC";
    acc_config.params.target_comp_id = "INIT";
    acc_config.params.heart_beat_int = 10;

    // First Handshake
    auto [c_init_1, c_acc_1] = TestSupport::LoopbackConnection::make_pair(io);
    factory.add_connection(c_init_1); // for initiator
    factory.add_connection(c_acc_1);  // for acceptor

    manager.create_session(init_config);
    manager.create_session(acc_config);

    for (int i = 0; i < 50; ++i) {
        pump(io);
        if (contains_fix_field(c_init_1->peek_write_log(), "35=A") && 
            contains_fix_field(c_acc_1->peek_write_log(), "35=A")) {
            break;
        }
    }

    EXPECT_TRUE(contains_fix_field(c_init_1->peek_write_log(), "35=A")) << "Init send Logon 1";
    EXPECT_TRUE(contains_fix_field(c_acc_1->peek_write_log(), "35=A")) << "Acc send Logon 1";

    // Second Handshake (Reconnect)
    auto [c_init_2, c_acc_2] = TestSupport::LoopbackConnection::make_pair(io);
    
    // We expect both sessions to try to reconnect/listen.
    // Order depends on implementation, but Init connects, Acc listens.
    // factory just pops. So we need to ensure the order of popping matches or it doesn't matter (since both c ends are same pipe).
    // Actually it matters because Init expects c_init_2 and Acc expects c_acc_2? No.
    // c_init_2 and c_acc_2 are peers.
    // If Init gets c_init_2 and Acc gets c_acc_2 (via listen), they are connected.
    // If Init gets c_acc_2 and Acc gets c_init_2, they are also connected (just swapped perspective, but LoopbackConnection is symmetric).
    // So order doesn't matter for the connection *pair*, but we must add *both*.
    
    factory.add_connection(c_init_2);
    factory.add_connection(c_acc_2);

    // Trigger disconnect on both ends
    c_init_1->simulate_disconnect();
    c_acc_1->simulate_disconnect();

    // Wait for reconnection
    int max_retries = 50;
    while (factory.calls.size() < 4 && max_retries-- > 0) { // 2 create calls + 2 reconnect calls
        pump(io);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    ASSERT_EQ(factory.calls.size(), 4);

    // Pump for handshake
    for (int i = 0; i < 50; ++i) {
        pump(io);
        if (contains_fix_field(c_init_2->peek_write_log(), "35=A") && 
            contains_fix_field(c_acc_2->peek_write_log(), "35=A")) {
            break;
        }
    }

    EXPECT_TRUE(contains_fix_field(c_init_2->peek_write_log(), "35=A")) << "Init send Logon 2 (reconnect)";
    EXPECT_TRUE(contains_fix_field(c_acc_2->peek_write_log(), "35=A")) << "Acc send Logon 2 (reconnect)";
}
