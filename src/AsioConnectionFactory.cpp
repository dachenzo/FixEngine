#include <chrono>
#include <random>
#include <memory>
#include <fix/IConnection.hpp>
#include <fix/error/Network.hpp>

using boost::asio::ip::tcp;

namespace Fix {

    AsioConnectionFactory::AsioConnectionFactory(boost::asio::io_context& ctx): io_{ctx} {}


    struct ResolveConnectOP: std::enable_shared_from_this<ResolveConnectOP> {
        
        boost::asio::io_context& io;
        tcp::resolver resolver;
        tcp::socket socket;
        boost::asio::steady_timer backoff_timer;

        std::string host;
        std::string service;
        Fix::IConnectionFactory::ConnectHandler cb;

        int attempt = 0;
        static const int max_attempts = 8;
        std::chrono::milliseconds base_delay{200};               // backoff start
        std::chrono::milliseconds max_delay{3000};               // cap
        std::chrono::milliseconds connect_timeout{5000};         // per-attempt connect timeout

        std::mt19937 rng{std::random_device{}()};
        std::uniform_real_distribution<> uni{0.0, 1.0};

        ResolveConnectOP(boost::asio::io_context& ctx, std::string h, std::string s, Fix::IConnectionFactory::ConnectHandler cb_)
        : io(ctx),
          resolver(ctx),
          socket(ctx),
          backoff_timer(ctx),
          host(std::move(h)),
          service(std::move(s)),
          cb(std::move(cb_)) {}

        
        std::chrono::milliseconds next_backoff() {
            auto pow = (1u << std::min(attempt, 12)); // avoid overflow
            auto raw = base_delay * pow;
            if (raw > max_delay) raw = max_delay;
            double j = 0.5 + 0.5 * uni(rng); // 0.5 .. 1.0
            return std::chrono::milliseconds(static_cast<int>(raw.count() * j));
        }


        void schedule_retry(std::function<void()> func) {
            if (++attempt > max_attempts) {
                cb(boost::asio::error::make_error_code(boost::asio::error::timed_out), {});
                return;
            }

            // Cancel any previously-armed backoff just in case
            boost::system::error_code ignore;
            backoff_timer.cancel(ignore);

            auto delay = next_backoff();
            backoff_timer.expires_after(delay);
            auto self = shared_from_this();
            backoff_timer.async_wait([self, fn = std::move(func)](const boost::system::error_code& ec){
                if (!ec) fn(); // timer not canceled
            });
        }
 
        
        void do_resolve() {
            auto self = shared_from_this();
            resolver.async_resolve(host, service, 
                [self](const boost::system::error_code& ec, tcp::resolver::results_type results) {
                    if (ec) {
                        if (ec == boost::asio::error::host_not_found_try_again) {
                            self->schedule_retry([self]{self->do_resolve();});
                            return;
                        }
                        self->cb(ec, {});
                        return;
                    }

                    if (results.empty()) {
                        self->cb(boost::asio::error::make_error_code(boost::asio::error::host_not_found), {});
                        return;
                    } 

                    self->do_connect(results);
                }
            
            );
        }

        void do_connect(const tcp::resolver::results_type& results) {
            auto self = shared_from_this();


            boost::asio::async_connect(socket, results, 
                [self](const boost::system::error_code& ec, const tcp::endpoint&) {
                    if (ec) {
                        Fix::Error::RetryClass errclass = Error::classify_connect_error(ec);
                        if (errclass == Error::RetryClass::Transient) {
                            boost::system::error_code ignore;
                            self->socket.close(ignore);                   // instead of cancel
                            self->socket = tcp::socket(self->io);         // fresh socket
                            self->schedule_retry([self]{ self->do_resolve(); });
                            return;
                        } else {
                            self->cb(ec, {});
                            return;
                        }
                    }

                    auto shared_sock = std::make_shared<tcp::socket>(std::move(self->socket));
                    auto conn = std::make_shared<AsioConnection>(std::move(shared_sock));
                    self->cb({}, std::move(conn));
                }
            );
        }

        void start() {
            do_resolve();
        }

        
    };

    void AsioConnectionFactory::async_connect(const Fix::ConnectionConfig& cfg, ConnectHandler handler) {  
        auto op = std::make_shared<ResolveConnectOP>(io_, std::string(cfg.ip), std::to_string(cfg.port), std::move(handler));
        op->start();
    }

    struct ListenOP: std::enable_shared_from_this<ListenOP> {
        using tcp = boost::asio::ip::tcp;
        

        boost::asio::io_context& io;
        tcp::acceptor acceptor;
        tcp::endpoint endpoint;
        Fix::IConnectionFactory::ConnectHandler cb;
        int backlog;

        int attempt = 0;
        static constexpr int max_attempts = 6;
        std::chrono::milliseconds base{200};
        std::chrono::milliseconds cap{3000};
        boost::asio::steady_timer backoff{io};

        ListenOP(
            boost::asio::io_context& ctx,
            tcp::endpoint ep,
            Fix::IConnectionFactory::ConnectHandler handler,
            int bl
        ): io{ctx}, acceptor{ctx}, endpoint{std::move(ep)}, cb{std::move(handler)},
        backlog(bl) {
             
        }

        void start() {
            try_listen();
        }


        std::chrono::milliseconds next_backoff() const {
            auto ms = base * (1 << std::min(attempt, 12));
            if (ms > cap) ms = cap;
            return ms;
        }

        void try_listen() {
            boost::system::error_code ec;
            acceptor = tcp::acceptor(io); // May not be necessary, just playing safe for now;
            acceptor.open(endpoint.protocol(), ec);

            if (ec) {
                if (is_transient_open(ec)) return schedule_retry(ec);
                cb(ec, {});
                return;
            } 

            acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);
            if (ec) {
                // usually config bug → abort
                return cb(ec, {});
            }


            acceptor.bind(endpoint, ec);
            if (ec) {
                if (is_transient_bind(ec)) return schedule_retry(ec);
                return cb(ec, {});
            }

             acceptor.listen(backlog, ec);
            if (ec) {
                if (is_transient_listen(ec)) return schedule_retry(ec);
                return cb(ec, {});
            }

            // success: start accepting
            do_accept();

        }


        void do_accept() {
            auto self = shared_from_this();
            acceptor.async_accept(
                [self](const boost::system::error_code& ec, tcp::socket sock) {
                    if (!ec) {
                        auto shared_sock = std::make_shared<tcp::socket>(std::move(sock));
                        auto conn = std::make_shared<AsioConnection>(std::move(shared_sock));
                        self->cb({}, std::move(conn));
                    } else {
                        self->cb(ec, {}); // loggin handles in call back already;
                    }
                }
            );
        }

        void schedule_retry(const boost::system::error_code& ec) {
            if (++attempt > max_attempts) {
                cb(ec, {}); // or last ec captured
                return;
            }
            backoff.expires_after(next_backoff());
            auto self = shared_from_this();
            backoff.async_wait([self](const boost::system::error_code& ec){
                if (!ec) self->try_listen();
            });
        }


        static bool is_transient_open(const boost::system::error_code& ec) {
            return ec == boost::system::errc::too_many_files_open ||
                ec == boost::system::errc::too_many_files_open_in_system;
        }
        static bool is_transient_bind(const boost::system::error_code& ec) {
            return ec == boost::asio::error::address_in_use; // TIME_WAIT / race with another process
        }
        static bool is_transient_listen(const boost::system::error_code& ec) {
            return ec == boost::system::errc::no_buffer_space; // system pressure
        }



    };
        
    void AsioConnectionFactory::async_listen(const Fix::ConnectionConfig& cfg,ConnectHandler handler) {
        const std::string host = cfg.ip.empty() ? std::string("0.0.0.0") : cfg.ip;
        const int backlog   = cfg.backlog > 0 ? cfg.backlog : SOMAXCONN;

        boost::system::error_code ec;
        auto addr = boost::asio::ip::make_address(host, ec);

        if (ec) { handler(ec, {}); return; }

        tcp::endpoint ep(addr, cfg.port);

        auto op = std::make_shared<ListenOP>(
            io_,
            ep,
            std::move(handler),
            backlog
        );

        op->start();
        
    }   
}
