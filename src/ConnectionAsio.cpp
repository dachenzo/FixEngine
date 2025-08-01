#include <boost/asio.hpp>
#include <memory>
#include <fix/IConnection.hpp>
#include <fix/definitions.hpp>




namespace Fix {

    AsioConnectionFactory::AsioConnectionFactory(boost::asio::io_context& io): io_{io} {}

    std::unique_ptr<IConnection> AsioConnectionFactory::make_acceptor (Fix::ConnectionConfig& config) {
        using namespace boost::asio;

        ip::tcp::resolver resolver(io_);
        auto endpoints = resolver.resolve(config.ip, std::to_string(config.port));

        auto socket = std::make_shared<ip::tcp::socket>(io_);
        boost::asio::connect(*socket, endpoints);

        return std::make_unique<AsioConnection>(std::move(socket));
        
    }


    std::unique_ptr<IConnection>
    AsioConnectionFactory::make_initiator (Fix::ConnectionConfig& config) {
        using namespace boost::asio;

        ip::tcp::endpoint endpoint(ip::make_address(config.ip), config.port);

        auto acceptor = std::make_shared<ip::tcp::acceptor>(io_, endpoint);

        auto socket = std::make_shared<ip::tcp::socket>(io_);

        acceptor->listen(1); //1 because its a 2 person connection so we only need a single person
        acceptor->accept(*socket);  // blocking

        return std::make_unique<AsioConnection>(std::move(socket));

    }

    std::unique_ptr<IConnection> AsioConnectionFactory::make_connection(
        Fix::ConnectionConfig& config
    ) {
        if (config.role == Role::ACCEPTOR) {return make_acceptor(config);}
        else {return make_initiator(config);}
    }


    AsioConnection::AsioConnection(std::shared_ptr<Socket> sockfd): sockfd_{sockfd} {}

    void AsioConnection::async_read_some(MutableBuffer& buffer, ReadHandler handle) {
        sockfd_->async_read_some(buffer, std::move(handle));
    }

    void AsioConnection::async_write_some(ConstBuffer& buffer, WriteHandler handle) {

    }

    
    void AsioConnection::close() {
        boost::system::error_code ec;

        if (sockfd_->is_open()) {
            sockfd_->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

            if (ec) {// handle error
            }

            sockfd_->close(ec);

            if (ec) {
                //handle error
            }
        }
    }


}