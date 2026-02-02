#include <gtest/gtest.h>
#include <boost/asio/io_context.hpp>
#include <fix/core/Application.hpp>
#include <fix/core/SessionPool.hpp>
#include <fix/log/LogCore.hpp>

namespace {
    struct DummyTimerFactory final : Fix::ITimerFactory {
        std::unique_ptr<Fix::ITimer> create_timer() override { return {}; }
    };
} // namespace

TEST(SessionPoolTests, EmplaceRemoveAndReuseSlot) {
    boost::asio::io_context io;
    Fix::AppSink app1{};
    Fix::AppSink app2{};
    Fix::AppSink app3{};
    DummyTimerFactory timers{};
    Fix::Log::LogCore log_core{"session_pool_tests"};

    Fix::SessionPool pool;
    Fix::SessionParameters p1{};
    p1.sender_comp_id = "S1";
    p1.target_comp_id = "T1";

    Fix::SessionParameters p2{};
    p2.sender_comp_id = "S2";
    p2.target_comp_id = "T2";

    Fix::ReconnectCallback cb = [](Fix::SessionID) {};

    auto s1 = pool.emplace_session(Fix::Role::INITIATOR, std::move(app1), timers, p1, log_core, io, cb);
    auto s2 = pool.emplace_session(Fix::Role::ACCEPTOR, std::move(app2), timers, p2, log_core, io, cb);

    ASSERT_NE(s1, nullptr);
    ASSERT_NE(s2, nullptr);
    EXPECT_EQ(pool.size(), 2u);

    const auto id1 = s1->get_session_id();
    const auto id2 = s2->get_session_id();

    EXPECT_NE(id1.id, id2.id);

    EXPECT_TRUE(pool.remove(id1));
    EXPECT_EQ(pool.size(), 1u);
    EXPECT_EQ(pool.get(id1), nullptr);
    EXPECT_NE(pool.get(id2), nullptr);

    // Reuse the removed slot.
    Fix::SessionParameters p3{};
    p3.sender_comp_id = "S3";
    p3.target_comp_id = "T3";
    auto s3 = pool.emplace_session(Fix::Role::INITIATOR, std::move(app3), timers, p3, log_core, io, cb);
    ASSERT_NE(s3, nullptr);
    EXPECT_EQ(pool.size(), 2u);

    // Storage index should be reused, but logical id should advance.
    EXPECT_EQ(s3->get_session_id().storage_index, id1.storage_index);
    EXPECT_NE(s3->get_session_id().id, id1.id);
}
