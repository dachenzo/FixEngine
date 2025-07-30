#pragma once 
#include <vector>
#include <string_view>
#include "IConnection.hpp"
#include "ITimer.hpp"
#include "MessageStore.hpp"
#include "Application.hpp"
#include "Session.hpp"
#include "definitions.hpp"

namespace Fix {
    struct SessionManager {

        SessionManager(Fix::Application& app, Fix::MessageStoreFactory store_factory, 
        Fix::IConnectionFactory& connFactory, Fix::ITimerFactory& timerFactory);


        Fix::Session& create_session();

        bool remove_session(const Fix::SessionID& id);

        Session* findSession(const Fix::SessionID& id);

        std::vector<Fix::SessionID> listSessions() const;

        void onConnectionReadable(const Fix::SessionID& id, std::string_view data);

        void onConnectionWritable(const Fix::SessionID& id);

        void onTimerEvent(const Fix::SessionID& id, Fix::TimerType which);

        void startAll();

        void stopAll();

        size_t sessionCount() const;


        

    };
}