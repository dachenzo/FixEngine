#include <boost/asio.hpp>
#include <memory>
#include <fix/IConnection.hpp>
#include <fix/definitions.hpp>




namespace Fix {

    AsioConnectionFactory::AsioConnectionFactory(boost::asio::io_context& io)
    : io_(io) {}

    struct ConnectOP: std::enable_shared_from_this<ConnectOP> {

        ConnectOP(boost::asio::io_context& io, IConnectionFactory::ConnectHandler cb): resolver{io}, socket{io}, callback{std::move(cb)}   {}

        boost::asio::ip::tcp::resolver resolver;
        boost::asio::ip::tcp::socket socket;
        IConnectionFactory::ConnectHandler callback;

    };


    void AsioConnectionFactory::async_connect(
        const Fix::ConnectionConfig& cfg,
        ConnectHandler handler
    ) {
        using boost::asio::ip::tcp;

        auto op = std::make_shared<ConnectOP>(io_, std::move(handler));

        // Adapt these two lines to your actual field names in ConnectionConfig
        const std::string host   = std::string(cfg.ip);     
        const std::string service = std::to_string(cfg.port); 


        op->resolver.async_resolve(host, service,
            [op](const boost::system::error_code& ec, tcp::resolver::results_type results) {
                if (ec) {op->callback(ec, {}); return;}

                boost::asio::async_connect(op->socket, results,
                    [op](const boost::system::error_code& ec, const tcp::endpoint&) {
                        if (ec) { op->callback(ec, {}); return; }

                        // Success: wrap the connected socket in our IConnection
                        auto conn = std::make_shared<AsioConnection>(std::move(op->socket));
                        op->callback({}, std::move(conn));
                    }
                );
            }
        );



    }

    using boost::asio::ip::tcp;
    struct ListenOp: std::enable_shared_from_this<ListenOp> {
        tcp::acceptor acceptor;
        tcp::endpoint endpoint;
        IConnectionFactory::ConnectHandler callback;
        int backlog;

        ListenOp(
            boost::asio::io_context& io,
            tcp::endpoint ep,
            IConnectionFactory::ConnectHandler cb,
            int blog
        ): acceptor(io), endpoint(ep), callback(std::move(cb)), backlog(blog) {}


        void start() {
            boost::system::error_code ec;
            acceptor.open(endpoint.protocol(), ec);

            if (ec) {callback(ec, {}); return;}

            acceptor.set_option(tcp::acceptor::reuse_address(true), ec);

            if (ec) {callback(ec, {}); return;}


            acceptor.bind(endpoint, ec);

            if(ec) {callback(ec, {}); return;}

            acceptor.listen(backlog, ec);

            if(ec) {callback(ec, {}); return;}

            do_accept();
        }

        void do_accept() {
            auto self = shared_from_this();
            acceptor.async_accept(
                [self](const boost::system::error_code& ec, tcp::socket sock) {
                
                    if (!ec) {
                        auto conn = std::make_shared<AsioConnection>(std::move(sock));
                        self->callback({}, std::move(conn));
                    } else {
                        self->callback(ec, {});
                    }

                    self->do_accept();
                }
            );
        }
    };




    void AsioConnectionFactory::async_listen(
        const Fix::ConnectionConfig& cfg,
        ConnectHandler handler
    ) {

        const std::string host = cfg.ip.empty() ? std::string("0.0.0.0") : cfg.ip;
        const int backlog   = cfg.backlog > 0 ? cfg.backlog : SOMAXCONN;

        boost::system::error_code ec;
        auto addr = boost::asio::ip::make_address(host, ec);

        if (ec) { handler(ec, {}); return; }

        tcp::endpoint ep(addr, cfg.port);

        auto op = std::make_shared<ListenOp>(
            io_,
            ep,
            std::move(handler),
            backlog
        );

        op->start();


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