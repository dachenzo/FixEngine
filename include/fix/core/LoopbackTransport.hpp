#pragma once

#include <boost/asio.hpp>
#include <cstring>
#include <mutex>
#include <optional>
#include <string>
#include <utility>
#include <fix/core/IConnection.hpp>

namespace Fix::TestSupport {

struct LoopbackChannel {
    struct PendingRead {
        boost::asio::mutable_buffer buffer;
        Fix::IConnection::ReadHandler handler;
        boost::asio::any_io_executor exec;
    };

    std::mutex mu;
    std::string inbound;
    std::optional<PendingRead> pending;
    bool closed = false;
};

// A minimal in-memory duplex connection suitable for driving Session logic in tests.
// - Writes append bytes into the peer's inbound buffer.
// - Reads complete when bytes are available (otherwise they pend).
// - close() marks this end as closed and aborts any pending read.
struct LoopbackConnection final : Fix::IConnection, public std::enable_shared_from_this<LoopbackConnection> {
    explicit LoopbackConnection(boost::asio::any_io_executor exec,
                               std::shared_ptr<LoopbackChannel> inbound,
                               std::shared_ptr<LoopbackChannel> outbound)
        : exec_{std::move(exec)}, inbound_{std::move(inbound)}, outbound_{std::move(outbound)} {}

    static std::pair<std::shared_ptr<LoopbackConnection>, std::shared_ptr<LoopbackConnection>> make_pair(boost::asio::io_context& io) {
        auto a_in = std::make_shared<LoopbackChannel>();
        auto b_in = std::make_shared<LoopbackChannel>();

        auto a = std::make_shared<LoopbackConnection>(io.get_executor(), a_in, b_in);
        auto b = std::make_shared<LoopbackConnection>(io.get_executor(), b_in, a_in);
        return {a, b};
    }

    void async_read_some(MutableBuffer& buffer, ReadHandler handle) override {
        auto self = shared_from_this();

        std::size_t n = 0;

        {
            std::lock_guard<std::mutex> lk(inbound_->mu);

            if (inbound_->closed) {
                boost::asio::post(exec_, [h = std::move(handle)]() mutable {
                    h(boost::asio::error::operation_aborted, 0);
                });
                return;
            }

            if (!inbound_->inbound.empty()) {
                n = std::min<std::size_t>(buffer.size(), inbound_->inbound.size());
                std::memcpy(buffer.data(), inbound_->inbound.data(), n);
                inbound_->inbound.erase(0, n);

                boost::asio::post(exec_, [h = std::move(handle), n]() mutable {
                    h({}, n);
                });
                return;
            }

            inbound_->pending = LoopbackChannel::PendingRead{buffer, std::move(handle), exec_};
        }
    }

    void async_write_some(ConstBuffer& buffer, WriteHandler handle) override {
        auto self = shared_from_this();

        const auto* data = static_cast<const char*>(buffer.data());
        const auto size = buffer.size();

        {
            std::lock_guard<std::mutex> lk(write_mu_);
            write_log_.append(data, size);
        }

        std::optional<LoopbackChannel::PendingRead> pending;
        {
            std::lock_guard<std::mutex> lk(outbound_->mu);

            if (outbound_->closed) {
                boost::asio::post(exec_, [h = std::move(handle)]() mutable {
                    h(boost::asio::error::broken_pipe, 0);
                });
                return;
            }

            outbound_->inbound.append(data, size);
            if (outbound_->pending.has_value()) {
                pending = std::move(outbound_->pending);
                outbound_->pending.reset();
            }
        }

        if (pending.has_value()) {
            // Fulfill the pending read on the reader's executor.
            std::size_t n = 0;
            {
                std::lock_guard<std::mutex> lk(outbound_->mu);
                n = std::min<std::size_t>(pending->buffer.size(), outbound_->inbound.size());
                std::memcpy(pending->buffer.data(), outbound_->inbound.data(), n);
                outbound_->inbound.erase(0, n);
            }

            boost::asio::post(pending->exec, [h = std::move(pending->handler), n]() mutable {
                h({}, n);
            });
        }

        boost::asio::post(exec_, [h = std::move(handle), size]() mutable {
            h({}, size);
        });
    }

    void close() override {
        std::optional<LoopbackChannel::PendingRead> pending;
        {
            std::lock_guard<std::mutex> lk(inbound_->mu);
            inbound_->closed = true;
            if (inbound_->pending.has_value()) {
                pending = std::move(inbound_->pending);
                inbound_->pending.reset();
            }
        }

        if (pending.has_value()) {
            boost::asio::post(pending->exec, [h = std::move(pending->handler)]() mutable {
                h(boost::asio::error::operation_aborted, 0);
            });
        }
    }

    boost::asio::any_io_executor get_executor() const override { return exec_; }

    std::string take_write_log() {
        std::lock_guard<std::mutex> lk(write_mu_);
        std::string out;
        out.swap(write_log_);
        return out;
    }

    std::string peek_write_log() const {
        std::lock_guard<std::mutex> lk(write_mu_);
        return write_log_;
    }

    // Inject bytes as if they arrived from the network into this end.
    void inject_inbound(std::string_view bytes) {
        std::optional<LoopbackChannel::PendingRead> pending;
        {
            std::lock_guard<std::mutex> lk(inbound_->mu);
            if (inbound_->closed) return;
            inbound_->inbound.append(bytes.data(), bytes.size());
            if (inbound_->pending.has_value()) {
                pending = std::move(inbound_->pending);
                inbound_->pending.reset();
            }
        }

        if (pending.has_value()) {
            std::size_t n = 0;
            {
                std::lock_guard<std::mutex> lk(inbound_->mu);
                n = std::min<std::size_t>(pending->buffer.size(), inbound_->inbound.size());
                std::memcpy(pending->buffer.data(), inbound_->inbound.data(), n);
                inbound_->inbound.erase(0, n);
            }

            boost::asio::post(pending->exec, [h = std::move(pending->handler), n]() mutable {
                h({}, n);
            });
        }
    }

private:
    boost::asio::any_io_executor exec_;
    std::shared_ptr<LoopbackChannel> inbound_;
    std::shared_ptr<LoopbackChannel> outbound_;

    mutable std::mutex write_mu_;
    std::string write_log_;
};

} // namespace Fix::TestSupport
