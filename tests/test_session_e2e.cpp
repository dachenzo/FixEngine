#include "fix/core/ApplicationEvents.hpp"
#include <gtest/gtest.h>

#include <boost/asio/io_context.hpp>

#include <fix/core/Application.hpp>
#include <fix/core/Clock.hpp>
#include <fix/core/MessageFactory.hpp>
#include <fix/core/SeqProvider.hpp>
#include <fix/core/Session.hpp>
#include <fix/log/LogCore.hpp>
#include <fix/core/LoopbackTransport.hpp>

namespace {
struct DummyTimerFactory final : Fix::ITimerFactory {
    std::unique_ptr<Fix::ITimer> create_timer() override { return {}; }
};

static void pump(boost::asio::io_context& io, int iters = 200) {
    for (int i = 0; i < iters; ++i) {
        const auto n = io.poll();
        if (n == 0) break;
    }
}

static bool contains_fix_field(std::string_view wire, std::string_view needle) {
    return wire.find(needle) != std::string_view::npos;
}

static constexpr const char* SOH = "\x01";
} // namespace

TEST(SessionE2E, InitiatorAndAcceptorCompleteLogonHandshake) {
    boost::asio::io_context io;
    Fix::AppSink app1{};
    Fix::AppSink app2{};
    DummyTimerFactory timers{};
    Fix::Log::LogCore log_core{"session_e2e_logon"};

    Fix::SessionID init_id{0, 1};
    Fix::SessionID acc_id{1, 2};

    Fix::SessionParameters init_params{};
    init_params.sender_comp_id = "CLT";
    init_params.target_comp_id = "SRV";
    init_params.heart_beat_int = 20;
    init_params.initiator_reset_on_logon = false;

    Fix::SessionParameters acc_params{};
    acc_params.sender_comp_id = "SRV";
    acc_params.target_comp_id = "CLT";
    acc_params.heart_beat_int = 20;
    acc_params.acceptor_reset_on_logon = true;

    Fix::ReconnectCallback cb = [](Fix::SessionID) {};

    log_core.add_session(init_id, "CLT<->SRV [1]");
    log_core.add_session(acc_id,  "SRV<->CLT [2]");

    auto initiator = std::make_shared<Fix::Session>(init_id, Fix::Role::INITIATOR, std::move(app1), timers, init_params, log_core, io, cb);
    auto acceptor  = std::make_shared<Fix::Session>(acc_id,  Fix::Role::ACCEPTOR,  std::move(app2), timers, acc_params,  log_core, io, cb);

    auto [c_init, c_acc] = Fix::TestSupport::LoopbackConnection::make_pair(io);

    initiator->set_connection(c_init);
    acceptor->set_connection(c_acc);

    initiator->start(Fix::StartMode::NORMAL);
    acceptor->start(Fix::StartMode::NORMAL);

    // Drive async writes/reads until acceptor replies with Logon.
    for (int i = 0; i < 200; ++i) {
        pump(io);
        if (contains_fix_field(c_acc->peek_write_log(), std::string(SOH) + "35=A" + SOH)) {
            break;
        }
    }

    const auto init_out = c_init->peek_write_log();
    const auto acc_out  = c_acc->peek_write_log();

    EXPECT_TRUE(contains_fix_field(init_out, std::string(SOH) + "35=A" + SOH));
    EXPECT_TRUE(contains_fix_field(acc_out,  std::string(SOH) + "35=A" + SOH));

    // Ensure HeartBtInt reflects heart_beat_int (MessageFactory logon should use the override/int).
    EXPECT_TRUE(contains_fix_field(init_out, std::string(SOH) + "108=20" + SOH));
    EXPECT_TRUE(contains_fix_field(acc_out,  std::string(SOH) + "108=20" + SOH));

    initiator->stop();
    acceptor->stop();
    pump(io);
}

TEST(SessionE2E, TestRequestGetsHeartbeatWithSameTestReqId) {
    boost::asio::io_context io;
    Fix::AppSink app1{};
    Fix::AppSink app2{};
    DummyTimerFactory timers{};
    Fix::Log::LogCore log_core{"session_e2e_testreq"};

    Fix::SessionID init_id{0, 1};
    Fix::SessionID acc_id{1, 2};

    Fix::SessionParameters init_params{};
    init_params.sender_comp_id = "CLT";
    init_params.target_comp_id = "SRV";
    init_params.heart_beat_int = 30;

    Fix::SessionParameters acc_params{};
    acc_params.sender_comp_id = "SRV";
    acc_params.target_comp_id = "CLT";
    acc_params.heart_beat_int = 30;

    Fix::ReconnectCallback cb = [](Fix::SessionID) {};

    log_core.add_session(init_id, "CLT<->SRV [1]");
    log_core.add_session(acc_id,  "SRV<->CLT [2]");

    auto initiator = std::make_shared<Fix::Session>(init_id, Fix::Role::INITIATOR, std::move(app1), timers, init_params, log_core, io, cb);
    auto acceptor  = std::make_shared<Fix::Session>(acc_id,  Fix::Role::ACCEPTOR,  std::move(app2), timers, acc_params,  log_core, io, cb);

    auto [c_init, c_acc] = Fix::TestSupport::LoopbackConnection::make_pair(io);

    initiator->set_connection(c_init);
    acceptor->set_connection(c_acc);
    initiator->start(Fix::StartMode::NORMAL);
    acceptor->start(Fix::StartMode::NORMAL);

    for (int i = 0; i < 200; ++i) {
        pump(io);
        if (contains_fix_field(c_acc->peek_write_log(), std::string(SOH) + "35=A" + SOH)) {
            break;
        }
    }

    // After handshake, acceptor expects next incoming seq = 2.
    Fix::SeqProvider sp;
    sp.update_out(2);
    Fix::Clock clock;
    Fix::MessageFactory<Fix::Clock> mf{init_params, sp, clock};

    const std::string test_req_id = "HELLO";
    const auto wire = std::string(mf.test_request(test_req_id));

    c_acc->inject_inbound(wire);
    pump(io);

    const auto acc_out = c_acc->peek_write_log();

    // Acceptor should respond with Heartbeat (35=0) including the same TestReqID (112).
    EXPECT_TRUE(contains_fix_field(acc_out, std::string(SOH) + "35=0" + SOH));
    EXPECT_TRUE(contains_fix_field(acc_out, std::string(SOH) + "112=HELLO" + SOH));

    initiator->stop();
    acceptor->stop();
    pump(io);
}


