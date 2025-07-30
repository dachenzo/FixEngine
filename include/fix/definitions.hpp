#include <cstdlib>

namespace Fix {
    using SessionID = std::size_t;

    enum class Role {
        INITIATOR,
        ACCEPTOR
    };

    enum class ConnectionEvent {
        Readable,   // socket is ready to read
        Writable,   // socket is ready to write queued data
        Closed,     // socket was closed or error
    };


    

    enum class TimerType {};

}