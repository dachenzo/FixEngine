#pragma once
#include <string_view>
#include <functional>
#include <memory>
#include <boost/asio.hpp>
#include <fix/definitions.hpp>


namespace Fix {

    struct IConnection {
        using ReadHandler  = std::function<void(boost::system::error_code, std::size_t)>;
        using WriteHandler = std::function<void(boost::system::error_code, std::size_t)>;

        using MutableBuffer = boost::asio::mutable_buffer;
        using ConstBuffer = boost::asio::const_buffer;
        virtual ~IConnection() = default;
        
        
        virtual void async_read_some(MutableBuffer& buffer, ReadHandler handle) = 0;
        
        virtual void async_write_some(ConstBuffer& buffer, WriteHandler handle) = 0;

        virtual void close() = 0;
    };

     

    struct IConnectionFactory {
        using ConnectHandler = std::function<void(const boost::system::error_code&, std::shared_ptr<IConnection>)>;

        virtual ~IConnectionFactory() = default;


        virtual void async_connect(const Fix::ConnectionConfig& cfg,
                                    ConnectHandler handler) = 0;

 
        virtual void async_listen(const Fix::ConnectionConfig& cfg,
                                    ConnectHandler handler) = 0;
};
    


    struct AsioConnection: IConnection {
        using Socket =  boost::asio::ip::tcp::socket;
        AsioConnection(std::shared_ptr<Socket> sockfd);

        void async_read_some(MutableBuffer& buffer, ReadHandler handle);
        
        void async_write_some(ConstBuffer& buffer, WriteHandler handle);

        void close();


        private:
        std::shared_ptr<Socket> sockfd_;
        
     
    };



    struct AsioConnectionFactory final : IConnectionFactory {
        explicit AsioConnectionFactory(boost::asio::io_context& io);

        void async_connect(const Fix::ConnectionConfig& cfg,
                            ConnectHandler handler) override;

        void async_listen(const Fix::ConnectionConfig& cfg,
                            ConnectHandler handler) override; 

        private:
        boost::asio::io_context& io_;
    };
}