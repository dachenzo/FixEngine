#include <memory>
#include <fix/SessionManager.hpp>



namespace Fix {
    SessionManager::SessionManager(
        Fix::Application& app,  
        Fix::IConnectionFactory& connFactory, 
        Fix::ITimerFactory& timerFactory):
        app_{app}, 
        store_factory_{},
        connFactory_{connFactory_},
        timerFactory_{timerFactory},
        session_pool_{}
        {
        } 


    void SessionManager::create_session(Fix::SessionCreationConfig& config) {
        auto conn_ = connFactory_.make_connection(config.conn_config);
        session_pool_.emplace_session(
            config.role,
            std::move(conn_),
            app_,
            timerFactory_
        );
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