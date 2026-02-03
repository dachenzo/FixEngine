#include "fix/message/admin/Custom.hpp"
#include <gtest/gtest.h>
#include <boost/asio/io_context.hpp>
#include <fix/core/ApplicationEvents.hpp>
#include <fix/core/Application.hpp>
#include <fix/core/Clock.hpp>
#include <fix/core/MessageFactory.hpp>
#include <fix/core/SeqProvider.hpp>
#include <fix/core/Session.hpp>
#include <fix/log/LogCore.hpp>
#include <fix/core/LoopbackTransport.hpp>
#include <sys/stat.h>

namespace {


static void pump(boost::asio::io_context& io, int iters = 200) {
    for (int i = 0; i < iters; ++i) {
        const auto n = io.poll();
        if (n == 0) break;
    }
}

static bool contains_fix_field(std::string_view wire, std::string_view needle) {
    return wire.find(needle) != std::string_view::npos;
}

static constexpr std::string SOH = "\x01";




} // namespace

TEST(SessionE2E, InitiatorAndAcceptorCompleteLogonHandshake) {
    boost::asio::io_context io;
    Fix::AppSink app1{};
    Fix::AppSink app2{};
   
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

    log_core.add_session(init_id, "init<->acc [1]");
    log_core.add_session(acc_id,  "acc<->init [2]");

    auto initiator = std::make_shared<Fix::Session>(init_id, Fix::Role::INITIATOR, std::move(app1), init_params, log_core, io, cb);
    auto acceptor  = std::make_shared<Fix::Session>(acc_id,  Fix::Role::ACCEPTOR,  std::move(app2), acc_params,  log_core, io, cb);

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

    log_core.add_session(init_id, "init<->acc [1]");
    log_core.add_session(acc_id,  "acc<->init [2]");

    auto initiator = std::make_shared<Fix::Session>(init_id, Fix::Role::INITIATOR, std::move(app1), init_params, log_core, io, cb);
    auto acceptor  = std::make_shared<Fix::Session>(acc_id,  Fix::Role::ACCEPTOR,  std::move(app2), acc_params,  log_core, io, cb);

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

TEST(SessionE2E, ResendGapFillLogic) {
    boost::asio::io_context io;
    Fix::AppSink app1 = [](const Fix::InBoundAppEvent&){};
    Fix::AppSink app2 = [](const Fix::InBoundAppEvent&){};
    Fix::Log::LogCore log_core{"session_e2e_resend"};

    Fix::SessionID init_id{0, 1};
    Fix::SessionID acc_id{1, 2};

    Fix::SessionParameters init_params{};
    init_params.sender_comp_id = "init";
    init_params.target_comp_id = "acc";
    init_params.heart_beat_int = 60;

    Fix::SessionParameters acc_params{};
    acc_params.sender_comp_id = "acc";
    acc_params.target_comp_id = "init";
    acc_params.heart_beat_int = 60;

    Fix::ReconnectCallback cb = [](Fix::SessionID) {};

    log_core.add_session(init_id, "init<->acc [1]");
    log_core.add_session(acc_id,  "acc<->init [2]");

    auto initiator = std::make_shared<Fix::Session>(init_id, Fix::Role::INITIATOR, std::move(app1), init_params, log_core, io, cb);
    auto acceptor  = std::make_shared<Fix::Session>(acc_id,  Fix::Role::ACCEPTOR,  std::move(app2), acc_params,  log_core, io, cb);

    auto [c_init, c_acc] = Fix::TestSupport::LoopbackConnection::make_pair(io);

    initiator->set_connection(c_init);
    acceptor->set_connection(c_acc);

    initiator->start(Fix::StartMode::NORMAL);
    acceptor->start(Fix::StartMode::NORMAL);

    // 1. Wait for Logon Handshake
    for (int i = 0; i < 200; ++i) {
        pump(io);
        if (contains_fix_field(c_init->peek_write_log(), std::string(SOH) + "35=A" + SOH)) {
            break;
        }
    }

    // Current State (Initiator Outbound):
    // Seq 1: Logon (Admin, skippable)

    // 2. Send Application Message
    // 2. Send Application Message ( Custom)
    {
        Fix::GenericMessage<Fix::GenericField> msg{
            {"Payload", 9250}
        };
        Fix::OutBoundAppMsg out_msg{std::move(msg), init_id, Fix::Message::Custom::MsgType};
        initiator->send_from_app(std::move(out_msg));
    }
    pump(io);
    // Init Store: 1=Logon, 2=Custom

    // 3. Force Heartbeat
    Fix::SeqProvider sp;
    sp.update_out(3); // Acceptor sent Logon(1) + Custom(2). So Next is 3.
    Fix::Clock clock;
    Fix::MessageFactory<Fix::Clock> mf{acc_params, sp, clock};

    std::string test_req_wire = std::string(mf.test_request("TEST_REQ_1"));

    c_init->inject_inbound(test_req_wire);
    pump(io);
    
    // Init Store: 1=Logon, 2=Custom, 3=Heartbeat 

    c_init->take_write_log();
    
    // 4. Inject Resend (1-0)
    sp.update_out(4); // TestReq was 3. Next is 4.
    std::string resend_req_wire = std::string(mf.resend_request(1, 0));
    c_init->inject_inbound(resend_req_wire);
   
    pump(io);

    auto traffic = c_init->peek_write_log();

    // Verification:
    // 1. GapFill for Seq 1 (Logon) -> NewSeq 2
    //    MsgType=4 (SequenceReset), GapFillFlag=Y, MsgSeqNum=1, NewSeqNo=2
    EXPECT_TRUE(contains_fix_field(traffic, std::string(SOH) + "35=4" + SOH));
    EXPECT_TRUE(contains_fix_field(traffic, std::string(SOH) + "123=Y" + SOH));
    EXPECT_TRUE(contains_fix_field(traffic, std::string(SOH) + "34=1" + SOH));
    EXPECT_TRUE(contains_fix_field(traffic, std::string(SOH) + "36=2" + SOH));

    // 2. Resend Custom  (Seq 2)
    //    MsgType= Custom , MsgSeqNum=2, PossDup=Y
    EXPECT_TRUE(contains_fix_field(traffic, std::string(SOH) + std::string("35=") + std::string(Fix::Message::Custom::MsgType) + SOH));
    EXPECT_TRUE(contains_fix_field(traffic, std::string(SOH) + "34=2" + SOH));
    EXPECT_TRUE(contains_fix_field(traffic, std::string(SOH) + "43=Y" + SOH));

    // 3. GapFill for Seq 3 (Heartbeat)  -> NewSeq 4
    //    MsgType=4, GapFillFlag=Y, MsgSeqNum=3, NewSeqNo=4
    EXPECT_TRUE(contains_fix_field(traffic, std::string(SOH) + "34=3" + SOH));
    EXPECT_TRUE(contains_fix_field(traffic, std::string(SOH) + "36=4" + SOH));
    initiator->stop();
    acceptor->stop();
    pump(io);
}




TEST(SessionE2E, TriggerResendRequest) {
    // Placeholder for future test implementing resend request triggering.
    boost::asio::io_context io;
    Fix::AppSink app1 = [](const Fix::InBoundAppEvent&){};
    Fix::AppSink app2 = [](const Fix::InBoundAppEvent&){};
    Fix::Log::LogCore log_core{"session_e2e_resend"};

    Fix::SessionID init_id{0, 1};
    Fix::SessionID acc_id{1, 2};

    Fix::SessionParameters init_params{};
    init_params.sender_comp_id = "init";
    init_params.target_comp_id = "acc";
    init_params.heart_beat_int = 30;

    Fix::SessionParameters acc_params{};
    acc_params.sender_comp_id = "acc";
    acc_params.target_comp_id = "init";
    acc_params.heart_beat_int = 30;

    Fix::ReconnectCallback cb = [](Fix::SessionID) {};

    log_core.add_session(init_id, "init<->acc [1]");
    log_core.add_session(acc_id,  "acc<->init [2]");

    auto initiator = std::make_shared<Fix::Session>(init_id, Fix::Role::INITIATOR, std::move(app1), init_params, log_core, io, cb);
    auto acceptor  = std::make_shared<Fix::Session>(acc_id,  Fix::Role::ACCEPTOR,  std::move(app2), acc_params,  log_core, io, cb);

    auto [c_init, c_acc] = Fix::TestSupport::LoopbackConnection::make_pair(io);

    initiator->set_connection(c_init);
    acceptor->set_connection(c_acc);

    initiator->start(Fix::StartMode::NORMAL);
    acceptor->start(Fix::StartMode::NORMAL);

    // 1. Wait for Logon Handshake
    for (int i = 0; i < 200; ++i) {
        pump(io);
        if (contains_fix_field(c_init->peek_write_log(), std::string(SOH) + "35=A" + SOH)) {
            break;
        }
    }

    // Current State (Initiator Outbound):
    // Seq 1: Logon (Admin, skippable)

    // 2. Send Application Message
    // 2. Send Application Message ( Custom)
    {
        Fix::GenericMessage<Fix::GenericField> msg{
            {"Payload", 9250}
        };
        Fix::OutBoundAppMsg out_msg{std::move(msg), init_id, Fix::Message::Custom::MsgType};
        initiator->send_from_app(std::move(out_msg));
    }
    pump(io);
    // Init Store: 1=Logon, 2=Custom

    // 3. Force SeqNum too high 
    Fix::SeqProvider sp;
    sp.update_out(10); // Acceptor sent Logon(1) + Custom(2). So Next is 3 but use 10.
    Fix::Clock clock;
    Fix::MessageFactory<Fix::Clock> mf{acc_params, sp, clock};

    std::string test_req_wire = std::string(mf.custom_message("TEST_PAYLOAD"));
    
    c_init->inject_inbound(test_req_wire);
    c_init->take_write_log();
    pump(io);
    
    // Init Store: 1=Logon, 2=Custom, 10 = Custom

    auto traffic = c_init->peek_write_log();
    pump(io);


    
    // Verification:
    // 1. Resend(3, 9) should be sent
    
    EXPECT_TRUE(contains_fix_field(traffic, std::string(SOH) + "35=2" + SOH));
    EXPECT_TRUE(contains_fix_field(traffic, std::string(SOH) + "34=3" + SOH));
    EXPECT_TRUE(contains_fix_field(traffic, std::string(SOH) + "7=3" + SOH));
    EXPECT_TRUE(contains_fix_field(traffic, std::string(SOH) + "16=9" + SOH));

    initiator->stop();
    acceptor->stop();
    pump(io);
}


TEST(SessionE2E, ChunkedMessageHandling) {
    boost::asio::io_context io;
    Fix::AppSink app1 = [](const Fix::InBoundAppEvent&){};
    Fix::AppSink app2 = [](const Fix::InBoundAppEvent&){};
    Fix::Log::LogCore log_core{"session_e2e_resend"};

    Fix::SessionID init_id{0, 1};
    Fix::SessionID acc_id{1, 2};

    Fix::SessionParameters init_params{};
    init_params.sender_comp_id = "init";
    init_params.target_comp_id = "acc";
    init_params.heart_beat_int = 30;

    Fix::SessionParameters acc_params{};
    acc_params.sender_comp_id = "acc";
    acc_params.target_comp_id = "init";
    acc_params.heart_beat_int = 30;

    Fix::ReconnectCallback cb = [](Fix::SessionID) {};

    log_core.add_session(init_id, "init<->acc [1]");
    log_core.add_session(acc_id,  "acc<->init [2]");

    auto initiator = std::make_shared<Fix::Session>(init_id, Fix::Role::INITIATOR, std::move(app1), init_params, log_core, io, cb);
    auto acceptor  = std::make_shared<Fix::Session>(acc_id,  Fix::Role::ACCEPTOR,  std::move(app2), acc_params,  log_core, io, cb);

    auto [c_init, c_acc] = Fix::TestSupport::LoopbackConnection::make_pair(io);

    initiator->set_connection(c_init);
    acceptor->set_connection(c_acc);

    initiator->start(Fix::StartMode::NORMAL);
    acceptor->start(Fix::StartMode::NORMAL);

    // 1. Wait for Logon Handshake
    for (int i = 0; i < 200; ++i) {
        pump(io);
        if (contains_fix_field(c_init->peek_write_log(), std::string(SOH) + "35=A" + SOH)) {
            break;
        }
    }
    c_init->take_write_log();


    // 2. Send Chunked Message
    {
        Fix::SeqProvider sp;
        sp.update_out(2); // Acceptor sent Logon(1)  So Next is 2.
        Fix::Clock clock;
        Fix::MessageFactory<Fix::Clock> mf{acc_params, sp, clock};

        std::string custom_wire = std::string(mf.custom_message("TEST_PAYLOAD"));
        // Chunk into pieces
        const size_t chunk_size = 7;
        //Inject Garbage first
        c_init->inject_inbound("GARBAGE_DATA");
        pump(io);

        // Now inject in chunks
        for (size_t offset = 0; offset < custom_wire.size(); offset += chunk_size) {
            size_t len = std::min(chunk_size, custom_wire.size() - offset);
            c_init->inject_inbound(custom_wire.substr(offset, len));
            pump(io);
        }
    }
    pump(io);
    auto traffic = c_init->peek_write_log();
    // Verification:
    // 1. Ensure Custom Message processed correctly
    EXPECT_TRUE(contains_fix_field(traffic, std::string(SOH) + std::string("35=") + std::string(Fix::Message::Custom::MsgType) + SOH));
    EXPECT_TRUE(contains_fix_field(traffic, std::string(SOH) + "9250=custom_response")); 
    EXPECT_TRUE(contains_fix_field(traffic, std::string(SOH) + "34=2" + SOH));
    initiator->stop();
    acceptor->stop();
    pump(io);
}

 
TEST(SessionE2E, Logout) {
    boost::asio::io_context io;
    Fix::AppSink app1 = [](const Fix::InBoundAppEvent&){};
    Fix::AppSink app2 = [](const Fix::InBoundAppEvent&){};
    Fix::Log::LogCore log_core{"session_e2e_resend"};

    Fix::SessionID init_id{0, 1};
    Fix::SessionID acc_id{1, 2};

    Fix::SessionParameters init_params{};
    init_params.sender_comp_id = "init";
    init_params.target_comp_id = "acc";
    init_params.heart_beat_int = 30;

    Fix::SessionParameters acc_params{};
    acc_params.sender_comp_id = "acc";
    acc_params.target_comp_id = "init";
    acc_params.heart_beat_int = 30;

    Fix::ReconnectCallback cb = [](Fix::SessionID) {};

    log_core.add_session(init_id, "init<->acc [1]");
    log_core.add_session(acc_id,  "acc<->init [2]");

    auto initiator = std::make_shared<Fix::Session>(init_id, Fix::Role::INITIATOR, std::move(app1), init_params, log_core, io, cb);
    auto acceptor  = std::make_shared<Fix::Session>(acc_id,  Fix::Role::ACCEPTOR,  std::move(app2), acc_params,  log_core, io, cb);

    auto [c_init, c_acc] = Fix::TestSupport::LoopbackConnection::make_pair(io);

    initiator->set_connection(c_init);
    acceptor->set_connection(c_acc);

    initiator->start(Fix::StartMode::NORMAL);
    acceptor->start(Fix::StartMode::NORMAL);

    // 1. Wait for Logon Handshake
    for (int i = 0; i < 200; ++i) {
        pump(io);
        if (contains_fix_field(c_init->peek_write_log(), std::string(SOH) + "35=A" + SOH)) {
            break;
        }
    }


    {
        Fix::SeqProvider sp;
        sp.update_out(2); // Next MsgSeqNum is 2
        Fix::Clock clock;
        Fix::MessageFactory<Fix::Clock> mf{init_params, sp, clock};
        auto logout_wire = std::string(mf.logout("Client requested logout"));
        c_init->inject_inbound(logout_wire);
        c_init->take_write_log();
        pump(io);
    }


    auto traffic = c_init->peek_write_log();

    // Verification:
    // 1. Ensure Logout response sent
    EXPECT_TRUE(contains_fix_field(traffic, std::string(SOH) + "35=5" + SOH));

    initiator->stop();
    acceptor->stop();
    pump(io);
}


TEST(SessionE2E, IdleHeartbeat) {
    boost::asio::io_context io;
    Fix::AppSink app1 = [](const Fix::InBoundAppEvent&){};
    Fix::AppSink app2 = [](const Fix::InBoundAppEvent&){};
    Fix::Log::LogCore log_core{"session_e2e_idle_heartbeat"};

    Fix::SessionID init_id{0, 1};
    Fix::SessionID acc_id{1, 2};

    Fix::SessionParameters init_params{};
    init_params.sender_comp_id = "init";
    init_params.target_comp_id = "acc";
    init_params.heart_beat_int = 10;

    Fix::SessionParameters acc_params{};
    acc_params.sender_comp_id = "acc";
    acc_params.target_comp_id = "init";
    acc_params.heart_beat_int = 10;

    Fix::ReconnectCallback cb = [](Fix::SessionID) {};

    log_core.add_session(init_id, "init<->acc [1]");
    log_core.add_session(acc_id,  "acc<->init [2]");

    auto initiator = std::make_shared<Fix::Session>(init_id, Fix::Role::INITIATOR, std::move(app1), init_params, log_core, io, cb);
    auto acceptor  = std::make_shared<Fix::Session>(acc_id,  Fix::Role::ACCEPTOR,  std::move(app2), acc_params,  log_core, io, cb);

    auto [c_init, c_acc] = Fix::TestSupport::LoopbackConnection::make_pair(io);

    initiator->set_connection(c_init);
    acceptor->set_connection(c_acc);

    initiator->start(Fix::StartMode::NORMAL);
    acceptor->start(Fix::StartMode::NORMAL);

    // 1. Wait for Logon Handshake
    for (int i = 0; i < 200; ++i) {
        pump(io);
        if (contains_fix_field(c_init->peek_write_log(), std::string(SOH) + "35=A" + SOH)) {
            break;
        }
    }
    c_init->take_write_log();
    c_acc->take_write_log();

    // 2. Advance time to trigger heartbeat
    for (int i = 0; i < 12; ++i) {
        boost::asio::steady_timer timer(io, std::chrono::seconds(1));
        timer.wait();
        pump(io);   
    }

    auto init_traffic = c_init->peek_write_log();
    auto acc_traffic  = c_acc->peek_write_log();



    // Verification:    // 1. Ensure Heartbeats sent
    EXPECT_TRUE(contains_fix_field(init_traffic, std::string(SOH) + "35=0" + SOH));
    EXPECT_TRUE(contains_fix_field(init_traffic, std::string(SOH) + "34=2" + SOH));
    EXPECT_TRUE(contains_fix_field(acc_traffic,  std::string(SOH) + "35=0" + SOH));
    EXPECT_TRUE(contains_fix_field(acc_traffic,  std::string(SOH) + "34=2" + SOH));
    initiator->stop();
    acceptor->stop();
    pump(io);
}


TEST(SessionE2E, GapRecovery) {
    // we need to accertain that th session uses the recovery cache and actually recovers from the gap created by skipped messages
    boost::asio::io_context io;
    // AppSink not used for Custom messages as they are handled internally by Session
    Fix::AppSink app_acc = [](Fix::InBoundAppEvent&&){}; 

    Fix::Log::LogCore log_core{"session_e2e_gap_recovery"};
    Fix::SessionID acc_id{1, 2};

    Fix::SessionParameters acc_params{};
    acc_params.sender_comp_id = "SRV";
    acc_params.target_comp_id = "CLT";
    acc_params.heart_beat_int = 30;
    acc_params.acceptor_reset_on_logon = true; 

    Fix::ReconnectCallback cb = [](Fix::SessionID) {};
    log_core.add_session(acc_id,  "acc-[2]");

    auto acceptor  = std::make_shared<Fix::Session>(acc_id,  Fix::Role::ACCEPTOR,  std::move(app_acc), acc_params,  log_core, io, cb);

    // Use only c_acc to simulate incoming traffic
    auto [c_init, c_acc] = Fix::TestSupport::LoopbackConnection::make_pair(io);

    acceptor->set_connection(c_acc);
    acceptor->start(Fix::StartMode::NORMAL);

    // 1. Perform Logon
    Fix::SessionParameters init_params{};
    init_params.sender_comp_id = "CLT";
    init_params.target_comp_id = "SRV";
    init_params.heart_beat_int = 30;

    Fix::SeqProvider sp_client;
    sp_client.update_out(1); 
    Fix::Clock clock;
    Fix::MessageFactory<Fix::Clock> mf_client{init_params, sp_client, clock};

    std::string logon_wire = std::string(mf_client.logon(30, true));
    c_acc->inject_inbound(logon_wire);
    
    for (int i = 0; i < 200; ++i) {
        pump(io);
        if (contains_fix_field(c_acc->peek_write_log(), std::string(SOH) + "35=A" + SOH)) {
            break;
        }
    }
    c_acc->take_write_log(); // Clear log

    // 2. Create Gap: Send Seq 3 (skip 2)
    sp_client.update_out(3); 
    std::string msg3 = std::string(mf_client.custom_message("PAYLOAD_3"));
    c_acc->inject_inbound(msg3);
    pump(io);

    // 3. Verify Resend Request
    auto traffic = c_acc->take_write_log();
    // Expect ResendRequest (35=2) for Seq 2
    EXPECT_TRUE(contains_fix_field(traffic, std::string(SOH) + "35=2" + SOH));
    EXPECT_TRUE(contains_fix_field(traffic, std::string(SOH) + "7=2" + SOH));
    EXPECT_EQ(acceptor->get_state(), Fix::SessionState::RECOVERING_RESEND);

    // 4. Fill Gap: Send Seq 2
    Fix::SeqProvider sp_recovery;
    sp_recovery.update_out(2); 
    Fix::MessageFactory<Fix::Clock> mf_recovery{init_params, sp_recovery, clock};
    std::string msg2 = std::string(mf_recovery.custom_message("PAYLOAD_2"));
    
    c_acc->inject_inbound(msg2);
    pump(io);

    // 5. Verify Responses
    // If recovery works, Acceptor processes Msg 2 (Sending Response Seq 3)
    // AND processes Msg 3 from cache (Sending Response Seq 4).
    auto traffic2 = c_acc->peek_write_log();
    
    EXPECT_TRUE(contains_fix_field(traffic2, std::string(SOH) + "34=3" + SOH));
    EXPECT_TRUE(contains_fix_field(traffic2, std::string(SOH) + "34=4" + SOH));

    EXPECT_EQ(acceptor->get_state(), Fix::SessionState::ACTIVE);

    
    acceptor->stop();
    pump(io);
}

TEST(SessionE2E, SeqTooLowLogout) {
    boost::asio::io_context io;
    Fix::AppSink app1 = [](const Fix::InBoundAppEvent&){};
    Fix::AppSink app2 = [](const Fix::InBoundAppEvent&){};
    Fix::Log::LogCore log_core{"session_e2e_idle_heartbeat"};

    Fix::SessionID init_id{0, 1};
    Fix::SessionID acc_id{1, 2};

    Fix::SessionParameters init_params{};
    init_params.sender_comp_id = "init";
    init_params.target_comp_id = "acc";
    init_params.heart_beat_int = 10;

    Fix::SessionParameters acc_params{};
    acc_params.sender_comp_id = "acc";
    acc_params.target_comp_id = "init";
    acc_params.heart_beat_int = 10;

    Fix::ReconnectCallback cb = [](Fix::SessionID) {};

    log_core.add_session(init_id, "init<->acc [1]");
    log_core.add_session(acc_id,  "acc<->init [2]");

    auto initiator = std::make_shared<Fix::Session>(init_id, Fix::Role::INITIATOR, std::move(app1), init_params, log_core, io, cb);
    auto acceptor  = std::make_shared<Fix::Session>(acc_id,  Fix::Role::ACCEPTOR,  std::move(app2), acc_params,  log_core, io, cb);

    auto [c_init, c_acc] = Fix::TestSupport::LoopbackConnection::make_pair(io);

    initiator->set_connection(c_init);
    acceptor->set_connection(c_acc);

    initiator->start(Fix::StartMode::NORMAL);
    acceptor->start(Fix::StartMode::NORMAL);

    // 1. Wait for Logon Handshake
    for (int i = 0; i < 200; ++i) {
        pump(io);
        if (contains_fix_field(c_init->peek_write_log(), std::string(SOH) + "35=A" + SOH)) {
            break;
        }
    }

    c_init->take_write_log();

    {
        Fix::SeqProvider sp;
        sp.update_out(1); // Next MsgSeqNum is 2 but use 1 to trigger SeqTooLow
        Fix::Clock clock;
        Fix::MessageFactory<Fix::Clock> mf{init_params, sp, clock};
        auto custom_wire = std::string(mf.custom_message("SEQ_TOO_LOW"));
        c_init->inject_inbound(custom_wire);
        pump(io);
    }

    auto traffic = c_init->peek_write_log();
    // Verification:
    // 1. Ensure Logout sent
    EXPECT_TRUE(contains_fix_field(traffic, std::string(SOH) + "35=5" + SOH));
    

}

TEST(SessionE2E, MessageBeforeLogon) {
    boost::asio::io_context io;
    Fix::AppSink app1 = [](const Fix::InBoundAppEvent&){};
    Fix::AppSink app2 = [](const Fix::InBoundAppEvent&){};
    Fix::Log::LogCore log_core{"session_e2e_idle_heartbeat"};

    Fix::SessionID init_id{0, 1};
    Fix::SessionID acc_id{1, 2};

    Fix::SessionParameters init_params{};
    init_params.sender_comp_id = "init";
    init_params.target_comp_id = "acc";
    init_params.heart_beat_int = 10;

    Fix::SessionParameters acc_params{};
    acc_params.sender_comp_id = "acc";
    acc_params.target_comp_id = "init";
    acc_params.heart_beat_int = 10;

    Fix::ReconnectCallback cb = [](Fix::SessionID) {};

    log_core.add_session(init_id, "init<->acc [1]");
    log_core.add_session(acc_id,  "acc<->init [2]");

    auto initiator = std::make_shared<Fix::Session>(init_id, Fix::Role::INITIATOR, std::move(app1), init_params, log_core, io, cb);
    auto acceptor  = std::make_shared<Fix::Session>(acc_id,  Fix::Role::ACCEPTOR,  std::move(app2), acc_params,  log_core, io, cb);

    auto [c_init, c_acc] = Fix::TestSupport::LoopbackConnection::make_pair(io);

    initiator->set_connection(c_init);
    acceptor->set_connection(c_acc);

    initiator->start(Fix::StartMode::NORMAL);
    acceptor->start(Fix::StartMode::NORMAL);


    {
        Fix::SeqProvider sp;
        sp.update_out(1); // Next MsgSeqNum is 1
        Fix::Clock clock;
        Fix::MessageFactory<Fix::Clock> mf{init_params, sp, clock};
        auto custom_wire = std::string(mf.custom_message("MSG_BEFORE_LOGON"));
        c_acc->inject_inbound(custom_wire);
        pump(io);
    }

    // 1. Wait for Logon Handshake
    for (int i = 0; i < 200; ++i) {
        pump(io);
        if (contains_fix_field(c_init->peek_write_log(), std::string(SOH) + "35=A" + SOH)) {
            break;
        }
    }

    auto c_acc_traffic = c_acc->peek_write_log();
    auto c_init_traffic = c_init->peek_write_log();
    // Verification:
    // 1. Ensure Logout sent by Acceptor
    EXPECT_TRUE(contains_fix_field(c_acc_traffic, std::string(SOH) + "35=5" + SOH));
    // 2. Ensure Logout sent by Initiator
    EXPECT_TRUE(contains_fix_field(c_init_traffic, std::string(SOH) + "35=5" + SOH));
    

    initiator->stop();
    acceptor->stop();
    pump(io);
}

TEST(SessionE2E, RejectInvalidMessage) {
    boost::asio::io_context io;
    Fix::AppSink app1 = [](const Fix::InBoundAppEvent&){};
    Fix::AppSink app2 = [](const Fix::InBoundAppEvent&){};
    Fix::Log::LogCore log_core{"session_e2e_idle_heartbeat"};

    Fix::SessionID init_id{0, 1};
    Fix::SessionID acc_id{1, 2};

    Fix::SessionParameters init_params{};
    init_params.sender_comp_id = "init";
    init_params.target_comp_id = "acc";
    init_params.heart_beat_int = 10;

    Fix::SessionParameters acc_params{};
    acc_params.sender_comp_id = "acc";
    acc_params.target_comp_id = "init";
    acc_params.heart_beat_int = 10;

    Fix::ReconnectCallback cb = [](Fix::SessionID) {};

    log_core.add_session(init_id, "init<->acc [1]");
    log_core.add_session(acc_id,  "acc<->init [2]");

    auto initiator = std::make_shared<Fix::Session>(init_id, Fix::Role::INITIATOR, std::move(app1), init_params, log_core, io, cb);
    auto acceptor  = std::make_shared<Fix::Session>(acc_id,  Fix::Role::ACCEPTOR,  std::move(app2), acc_params,  log_core, io, cb);

    auto [c_init, c_acc] = Fix::TestSupport::LoopbackConnection::make_pair(io);

    initiator->set_connection(c_init);
    acceptor->set_connection(c_acc);

    initiator->start(Fix::StartMode::NORMAL);
    acceptor->start(Fix::StartMode::NORMAL);

    // 1. Wait for Logon Handshake
    for (int i = 0; i < 200; ++i) {
        pump(io);
        if (contains_fix_field(c_init->peek_write_log(), std::string(SOH) + "35=A" + SOH)) {
            break;
        }
    }
    c_acc->take_write_log();

    {
        Fix::SeqProvider sp;
        sp.update_out(2); // Next MsgSeqNum is 2
        Fix::Clock clock;
        Fix::MessageFactory<Fix::Clock> mf{init_params, sp, clock};
        // Create invalid message (missing required field)
        Fix::GenericMessage<Fix::GenericField> invalid_msg {
            {"InavlidField", 900} // Assuming 900 is not a valid field in custom message
        };
        
        std::string invalid_wire = std::string(mf.custom_message("INVALID_FIELD"));
        auto pos = invalid_wire.find("9250="); // Assuming 9250 is a required field
        invalid_wire[pos] = '8'; // Corrupt the field tag to make it invalid
        invalid_wire[pos+1] = '3'; // Corrupt the field tag to make it invalid but preserve bodylenght and checksum
        c_acc->inject_inbound(invalid_wire);
        pump(io);
    }


    auto traffic = c_acc->peek_write_log();
    // Verification:
    // 1. Ensure Reject sent
    EXPECT_TRUE(contains_fix_field(traffic, std::string(SOH) + "35=3" + SOH));
    EXPECT_TRUE(contains_fix_field(traffic, std::string(SOH) + "371=9250" + SOH)); //assuming 9250 is missing required field of custom message


    initiator->stop();
    acceptor->stop();
    pump(io);


}

TEST(SessionE2E, MultipleMessagesInSingleRead) {
    boost::asio::io_context io;
    Fix::AppSink app1 = [](const Fix::InBoundAppEvent&){};
    Fix::AppSink app2 = [](const Fix::InBoundAppEvent&){};
    Fix::Log::LogCore log_core{"session_e2e_idle_heartbeat"};

    Fix::SessionID init_id{0, 1};
    Fix::SessionID acc_id{1, 2};

    Fix::SessionParameters init_params{};
    init_params.sender_comp_id = "init";
    init_params.target_comp_id = "acc";
    init_params.heart_beat_int = 10;

    Fix::SessionParameters acc_params{};
    acc_params.sender_comp_id = "acc";
    acc_params.target_comp_id = "init";
    acc_params.heart_beat_int = 10;

    Fix::ReconnectCallback cb = [](Fix::SessionID) {};

    log_core.add_session(init_id, "init<->acc [1]");
    log_core.add_session(acc_id,  "acc<->init [2]");

    auto initiator = std::make_shared<Fix::Session>(init_id, Fix::Role::INITIATOR, std::move(app1), init_params, log_core, io, cb);
    auto acceptor  = std::make_shared<Fix::Session>(acc_id,  Fix::Role::ACCEPTOR,  std::move(app2), acc_params,  log_core, io, cb);

    auto [c_init, c_acc] = Fix::TestSupport::LoopbackConnection::make_pair(io);

    initiator->set_connection(c_init);
    acceptor->set_connection(c_acc);

    initiator->start(Fix::StartMode::NORMAL);
    acceptor->start(Fix::StartMode::NORMAL);

    // 1. Wait for Logon Handshake
    for (int i = 0; i < 200; ++i) {
        pump(io);
        if (contains_fix_field(c_init->peek_write_log(), std::string(SOH) + "35=A" + SOH)) {
            break;
        }
    }

    c_init->take_write_log();

    {
        Fix::SeqProvider sp;
        sp.update_out(2); // Next MsgSeqNum is 2
        Fix::Clock clock;
        Fix::MessageFactory<Fix::Clock> mf{acc_params, sp, clock};
        std::string msg1 = std::string(mf.custom_message("PAYLOAD_1"));
        sp.update_out(3);
        std::string msg2 = std::string(mf.custom_message("PAYLOAD_2"));
        sp.update_out(4);
        std::string msg3 = std::string(mf.custom_message("PAYLOAD_3"));

        // Concatenate messages to simulate single read
        std::string combined = msg1 + msg2 + msg3;
        
        c_init->inject_inbound(combined);
        pump(io);
        pump(io);
    }

    auto traffic = c_init->peek_write_log();
    
    // Verification:
    // 1. Ensure all three messages processed
    EXPECT_TRUE(contains_fix_field(traffic, std::string(SOH) + "34=2" + SOH));
    EXPECT_TRUE(contains_fix_field(traffic, std::string(SOH) + "34=3" + SOH));
    EXPECT_TRUE(contains_fix_field(traffic, std::string(SOH) + "34=4" + SOH));



}