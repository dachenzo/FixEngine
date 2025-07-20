#include <string.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <memory>
#include <string_view>


const std::string remote_host = "localhost";
const std::string remote_port = "30000";


namespace TCP {

    struct Connection{
        Connection(int sockfd): sockfd_{sockfd} {}

        std::string_view read(size_t len) {
            ssize_t n = recv(sockfd_, buff_, sizeof(buff_)-1, 0);
            return std::string_view{buff_, n};
        }

        ssize_t write(char* buff, std::size_t n) {
            ssize_t n = send(sockfd_, buff, n, 0);
            
        }

        private:
        int sockfd_;
        char buff_[8192];

    };



    struct Server {

        Server() {
            make_bind_socket_();
        }

        std::unique_ptr<TCP::Connection> acceptConnection() {
            int newfd = accept_(sockfd_);
            if (newfd == -1) {} //do something
            return std::make_unique<TCP::Connection>(newfd);
        }
        
        private:
        int sockfd_{-1};
        
        //FIXME: I need to handle the error situations, probably make a logging class
        void make_bind_socket_() {
            struct addrinfo *res{nullptr}, hints{};
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_family = AF_UNSPEC;
            hints.ai_family = AI_PASSIVE;
            int err = getaddrinfo(nullptr, "30000", &hints, &res);
            if (err != 0) {
                std::cerr << "[Server] getaddrinfo: " << gai_strerror(err) << "\n";
                
            }

            int sockfd = -1;
            for (auto p = res; p != nullptr; p = p->ai_next) {
                sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

                if (sockfd != -1) {
                    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == 0) break;
                    ::close(sockfd);
                    sockfd = -1;
                }
            }
            freeaddrinfo(res);
            if (sockfd == -1) {
                std::cerr << "[Server] Failed to bind any socket\n";
                
            }
            if (listen(sockfd, 3) == -1) {
                std::cerr << "[Server] listen failed\n";
                ::close(sockfd);
                
            }
            std::cout << "[Server] Listening on port 30000...\n";
            sockfd_ = sockfd;
        }

        int accept_(int sockfd) {
            struct sockaddr_storage their_addr;
            socklen_t addr_size = sizeof(their_addr);
            int new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &addr_size);
            if (new_fd == -1) {
                std::cerr << "[Server] accept failed\n";
                return -1;
            }
            return new_fd;
        }

    };

}