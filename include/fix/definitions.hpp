#pragma once
#include <cstdlib>

namespace Fix {
    
    struct SessionID {
        std::size_t storage_index;
        std::size_t id;

        bool operator==(const Fix::SessionID other) {
            return other.id == this->id;
        }

        bool operator!=(const Fix::SessionID other) {
            return other.id != this->id;
        }
    };

    enum class Role {
        INITIATOR,
        ACCEPTOR
    };

    struct ConnectionConfig {
        std::string ip;
        uint16_t port;
        Fix::Role role;
    }; 

    struct SessionCreationConfig {
        Fix::Role role;
        Fix::ConnectionConfig conn_config;
    };


    enum class ConnectionEvent {
        Readable,   // socket is ready to read
        Writable,   // socket is ready to write queued data
        Closed,     // socket was closed or error
    };


    

    enum class TimerType {};

}