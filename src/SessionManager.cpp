#include <boost/asio/io_context.hpp>
#include <memory>
#include <iostream>
#include <boost/asio.hpp>
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
        connFactory_{connFactory},
        timerFactory_{timerFactory},
        session_pool_{},
        io_context_{io_context},
        log_core_{generate_engine_id()}
        {
        } 


    void SessionManager::create_session(const Fix::SessionCreationConfig& config) {
        auto sess = session_pool_.emplace_session(
            config.role,
            app_,
            timerFactory_,
            config.params,
            log_core_,
            io_context_
        );
        auto id = sess->get_session_id();
        log_core_.add_session(id, sess->readable_id());

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
                    s->start();
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
                        return;
                    }

                    if (auto s = w.lock()) {
                        s->set_connection(std::move(conn));
                        s->start();
                    }
                }
            );

        }

        
    }

    void SessionManager::create_all(std::vector<Fix::SessionCreationConfig>& confgs) {
        for (Fix::SessionCreationConfig& config: confgs) { create_session(config);}
    }    

    void SessionManager::start_all() {
        for (auto sess: session_pool_.get_all_at_start()) {sess->start();}
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