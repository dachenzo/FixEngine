#include <memory>
#include <iostream>
#include <boost/asio.hpp>
#include <fix/SessionManager.hpp>



namespace Fix {
    SessionManager::SessionManager(
        Fix::Application& app,  
        Fix::IConnectionFactory& connFactory, 
        Fix::ITimerFactory& timerFactory):
        app_{app}, 
        connFactory_{connFactory},
        timerFactory_{timerFactory},
        session_pool_{}
        {
        } 


    void SessionManager::create_session(const Fix::SessionCreationConfig& config) {
        auto sess = session_pool_.emplace_session(
            config.role,
            app_,
            timerFactory_,
            config.params
        );
        std::cout << "Session Created\n";
        if (config.role == Fix::Role::ACCEPTOR) {
           
            connFactory_.async_connect(config.conn_config,
            [w = std::weak_ptr<Fix::Session>(sess)](const boost::system::error_code& ec,
            std::shared_ptr<IConnection> conn) {
                if (ec) {
                    // log + schedule retry/backoff here
                    return;
                }
 
                if (auto s = w.lock()) {
                    s->set_connection(std::move(conn));
                    s->start();
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
            sess->stop();
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