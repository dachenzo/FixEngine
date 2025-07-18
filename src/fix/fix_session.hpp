
#include "fix_parser.hpp"
#include "net/tcp_connection.hpp"

namespace Fix {

    struct Session {

        private:
        Fix::Parser parser;
        TCP::Connection conn;

    };
};