#include <fix/core/IConnection.hpp>

namespace Fix {
    
    AsioConnection::AsioConnection(std::shared_ptr<Socket> sockfd): sockfd_{std::move(sockfd)} {}

    void AsioConnection::async_read_some(MutableBuffer& buffer, ReadHandler handle) {
        sockfd_->async_read_some(buffer, std::move(handle));
    }

    void AsioConnection::async_write_some(ConstBuffer& buffer, WriteHandler handle) {
        sockfd_->async_write_some(buffer, std::move(handle));
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

    boost::asio::any_io_executor AsioConnection::get_executor() const {
        return sockfd_->get_executor();
    }
}