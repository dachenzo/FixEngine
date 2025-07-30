#include <boost/asio.hpp>
#include <memory>
#include "include/fix/IConnection.hpp"
#include "include/fix/definitions.hpp"




namespace Fix {

    AsioConnectionFactory::AsioConnectionFactory(boost::asio::io_context& io): io_{io} {}

    std::shared_ptr<IConnection> AsioConnectionFactory::make_acceptor (Fix::ConnectionConfig& config, Fix::SessionID session_id) {
        using namespace boost::asio;

        ip::tcp::resolver resolver(io_);
        auto endpoints = resolver.resolve(config.ip, std::to_string(config.port));

        auto socket = std::make_shared<ip::tcp::socket>(io_);
        boost::asio::connect(*socket, endpoints);

        return std::make_shared<AsioConnection>(std::move(socket), session_id);
        
    }


    std::shared_ptr<IConnection>
    AsioConnectionFactory::make_initiator (Fix::ConnectionConfig& config, Fix::SessionID session_id) {
        using namespace boost::asio;

        ip::tcp::endpoint endpoint(ip::make_address(config.ip), config.port);

        auto acceptor = std::make_shared<ip::tcp::acceptor>(io_, endpoint);

        auto socket = std::make_shared<ip::tcp::socket>(io_);

        acceptor->listen(1); //1 because its a 2 person connection so we only need a single person
        acceptor->accept(*socket);  // blocking

        return std::make_shared<AsioConnection>(std::move(socket), session_id);

    }


    AsioConnection::AsioConnection(std::shared_ptr<Socket> sockfd, Fix::SessionID sessionId): sockfd_{sockfd}, session_id_{sessionId} {}

    void AsioConnection::async_read_some(MutableBuffer& buffer, ReadHandler& handle) {
        sockfd_->async_read_some(buffer, std::move(handle));
    }

    
    
    void AsioConnection::close() {}

    Fix::SessionID AsioConnection::session_id() const {}
}