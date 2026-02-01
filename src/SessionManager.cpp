#include "fix/core/Session.hpp"
#include <boost/asio/io_context.hpp>
#include <memory>
#include <iostream>
#include <boost/asio.hpp>
#include <unordered_map>
#include <fix/core/SessionManager.hpp>
#include <fix/core/utils.hpp>



namespace Fix {

    std::string SessionManager::generate_engine_id() {
        auto now = std::chrono::system_clock::now();
        std::string id = std::format("{:%Y-%m-%d %H:%M:%S}", 
                                        std::chrono::floor<std::chrono::seconds>(now));
        id.push_back('-');
        id += std::to_string(Fix::Utils::get_pid());
        return id;
        
    }

    SessionManager::SessionManager(
        Fix::Application& app,  
        Fix::IConnectionFactory& connFactory, 
        Fix::ITimerFactory& timerFactory,
        boost::asio::io_context& io_context):
        app_{app}, 
        exec_{boost::asio::make_strand(io_context)},
        connFactory_{connFactory},
        timerFactory_{timerFactory},
        session_pool_{},
        io_context_{io_context},
        log_core_{generate_engine_id()}
        {
        } 


    void SessionManager::create_session(const Fix::SessionCreationConfig& config) {

        ReconnectCallback reconnect_callback =
        [mgr_exec = exec_, this](Fix::SessionID id) {
            // it is safe to capture this because SessionManager's lifetime is the engine's lifetime
            boost::asio::post(mgr_exec, [this, id]{
                this->reconnect_session_impl_(id);
            });
        };

        auto sess = session_pool_.emplace_session(
            config.role,
            app_,
            timerFactory_,
            config.params,
            log_core_,
            io_context_,
            reconnect_callback
        );
        auto id = sess->get_session_id();
        log_core_.add_session(id, sess->readable_id());
        session_configs_.emplace(id, config);

        
        std::cout << "Session Created\n";
        if (config.role == Fix::Role::ACCEPTOR) {
           
            connFactory_.async_connect(config.conn_config,
            [w = std::weak_ptr<Fix::Session>(sess)](const boost::system::error_code& ec,
            std::shared_ptr<IConnection> conn) {
                if (ec) {
                    if (auto s = w.lock()) {
                        s->logger().log(
                            {
                                Fix::Error::Layer::Transport,
                                Fix::Error::Category::Error,
                                Fix::Error::Severity::High
                            },
                            "Failed to connect: " + ec.message()
                        );
                    }
                    return;
                }
 
                if (auto s = w.lock()) {
                    s->set_connection(std::move(conn));
                    s->start(StartMode::NORMAL);
                } else {
                    conn->close();
                }
            }
            );
        } else {
            
            connFactory_.async_listen(
                config.conn_config,
                [w = std::weak_ptr<Session>(sess)]
                (const boost::system::error_code ec,
                std::shared_ptr<IConnection> conn
                ) {
                    if (ec) {
                        if (auto s = w.lock()) {
                            s->logger().log(
                                {
                                    Fix::Error::Layer::Transport,
                                    Fix::Error::Category::Error,
                                    Fix::Error::Severity::High
                                },
                                "Failed to listen for incoming connection: " + ec.message()
                            );
                        }
                        return;
                    }

                    if (auto s = w.lock()) {
                        s->set_connection(std::move(conn));
                        s->start(StartMode::NORMAL);
                    }
                }
            );

        }

        
    }

    void SessionManager::reconnect_session_impl_(const Fix::SessionID& id) {
        auto it = session_configs_.find(id);
        if (it == session_configs_.end()) {
            return;
        }
        auto sess = session_pool_.get(id);
        if (!sess) {
            return;
        }
        auto& config = it->second;

        if (config.role == Fix::Role::ACCEPTOR) {
            connFactory_.async_connect(config.conn_config,
            [w = std::weak_ptr<Fix::Session>(sess)](const boost::system::error_code& ec,
            std::shared_ptr<IConnection> conn) {
                if (ec) {
                    if (auto s = w.lock()) {
                        s->logger().log(
                            {
                                Fix::Error::Layer::Transport,
                                Fix::Error::Category::Error,
                                Fix::Error::Severity::High
                            },
                            "Failed to reconnect after disconnect: " + ec.message()
                        );
                    }
                    return;
                }

                if (auto s = w.lock()) {
                    s->set_connection(std::move(conn));
                    s->start(StartMode::RECONNECT);
                } else {
                    conn->close();
                }
            }
            );
        } else {
            
            connFactory_.async_listen(
                config.conn_config,
                [w = std::weak_ptr<Session>(sess)]
                (const boost::system::error_code ec,
                std::shared_ptr<IConnection> conn
                ) {
                    if (ec) {
                        if (auto s = w.lock()) {
                            s->logger().log(
                                {
                                    Fix::Error::Layer::Transport,
                                    Fix::Error::Category::Error,
                                    Fix::Error::Severity::High
                                },
                                "Failed to listen for incoming connection during reconnect: " + ec.message()
                            );
                        }
                        return;
                    }

                    if (auto s = w.lock()) {
                        s->set_connection(std::move(conn));
                        s->start(StartMode::RECONNECT);
                    }
                }
            );

        }
    }


    void SessionManager::send(OutBoundAppMsg&& msg) {
        boost::asio::post(exec_,
        [this, m = std::move(msg)]() mutable {
            auto session = session_pool_.get(m.session_id);
            if (!session) {
                // session not found, the manager needs logging
                return;
            }

            session->send_from_app(std::move(m));

            
        });
    }
        
    

    void SessionManager::create_all(std::vector<Fix::SessionCreationConfig>& confgs) {
        session_configs_.reserve(confgs.size());
        for (Fix::SessionCreationConfig& config: confgs) { create_session(config);}
    }    

    void SessionManager::start_all() {
        for (auto sess: session_pool_.get_all_at_start()) {sess->start(StartMode::NORMAL);}
    }

    void SessionManager::stop_all() {
        for (auto sess: session_pool_.get_all()) {
            if (sess) sess->stop();
        }
    }

    bool SessionManager::remove_session(const Fix::SessionID& id) {
        return session_pool_.remove(id);
    }

    std::shared_ptr<Fix::Session> SessionManager::findSession(const Fix::SessionID& id) {
        return session_pool_.get(id);
    }

    std::size_t SessionManager::sessionCount()  noexcept {
        return session_pool_.size();
    }


}