#pragma once
#include <string_view>
#include <functional>
#include <memory>
#include "definitions.hpp"


namespace Fix {

    struct ConnectionConfig {
        std::string ip;
        uint16_t port;
    };  

    struct IConnectionFactory {
        
        virtual ~IConnectionFactory() = default;

        virtual std::shared_ptr<IConnection> make_initiator(Fix::ConnectionConfig& config, Fix::SessionID session_id) = 0;

        virtual std::shared_ptr<IConnection> make_acceptor(Fix::ConnectionConfig& config, Fix::SessionID session_id) = 0;


    };

    struct IConnection {
       using ReadHandler  = std::function<void(boost::system::error_code, std::size_t)>;
        using WriteHandler = std::function<void(boost::system::error_code, std::size_t)>;

        using MutableBuffer = boost::asio::mutable_buffer;
    
        virtual ~IConnection() = default;
        
        
        virtual void async_read_some(MutableBuffer& buffer, ReadHandler& handle) = 0;
        
        virtual void async_write_some(const char * buffer, WriteHandler& handle) = 0;

        virtual void close() = 0;
    };


    struct AsioConnection: IConnection {
        using Socket =  boost::asio::ip::tcp::socket;
        AsioConnection::AsioConnection(std::shared_ptr<Socket> sockfd, Fix::SessionID session_id);

         void async_read_some(MutableBuffer& buffer, ReadHandler& handle);
        
         void async_write_some(const char * buffer, WriteHandler handle);

        void close();

        Fix::SessionID session_id() const;

        private:
        std::shared_ptr<Socket> sockfd_;
        Fix::SessionID session_id_;
     
    };


    struct AsioConnectionFactory: IConnectionFactory {

        AsioConnectionFactory(boost::asio::io_context& io);
        
        std::shared_ptr<IConnection> make_initiator(Fix::ConnectionConfig& config,Fix::SessionID session_id
        );

        std::shared_ptr<IConnection> make_acceptor(Fix::ConnectionConfig& config, Fix::SessionID session_id);

        private:
        boost::asio::io_context& io_;

    };
}