#include "fix/core/Application.hpp"
#include <vector>
#include <iostream>
#include <fix/cli/FixShell.hpp>
#include <fix/core/SessionManager.hpp>
#include <fix/core/IConnection.hpp>
#include <fix/core/definitions.hpp>
#include <fix/core/Reactor.hpp>
#include <fix/message/GenericMessage.hpp>
#include <fix/core/ApplicationEvents.hpp>



struct ApplicationExample {
    ApplicationExample(Fix::SessionManager* session_manager, boost::asio::io_context& io_context):
        exec_{boost::asio::make_strand(io_context.get_executor())},
        session_manager_{session_manager}
        {}
        std::string get_name() { return "ApplicationExample"; }
        Fix::AppSink get_app_sink() { 
            return [this](Fix::InBoundAppEvent&& event) {
                boost::asio::post(exec_, [this, ev = std::move(event)]() mutable {
                    process_event_(std::move(ev));
                });
            }; 
        }

        void set_session_manager(Fix::SessionManager* mgr) {
            session_manager_ = mgr;
        }

        void set_session_ids(std::vector<Fix::SessionID>&& ids) {
            session_ids_ = std::move(ids);
        }

        private:
        void process_event_(Fix::InBoundAppEvent&& event) {
            // assume in strand already
            std::cout << "ApplicationExample received message of type: " 
                      << *event.valid_message.header_cache_.slots[static_cast<size_t>(Fix::CacheSlot::MsgType)] 
                      << " for session: " << event.session_id.id << std::endl;
            Fix::GenericMessage<Fix::GenericField> msg{{"Yello", 9250}};
            Fix::OutBoundAppMsg out_msg{std::move(msg), event.session_id, Fix::Message::Custom::MsgType};
            session_manager_->send(std::move(out_msg));  

        }

        boost::asio::strand<boost::asio::any_io_executor> exec_;
        std::vector<Fix::SessionID> session_ids_;
        Fix::SessionManager* session_manager_;
};



int main () {

     Fix::ConnectionConfig server_conn_config {
        "127.0.0.1",
        5001,
        Fix::Role::ACCEPTOR,
        1
    };

    Fix::SessionParameters server_session_params {
        .sender_comp_id = "SRV",
        .target_comp_id = "CLT",
        .heart_beat_int = 30
    };

    
    Fix::SessionCreationConfig server_config {
        server_conn_config.role,
        server_conn_config,
        server_session_params
    };

    Fix::ConnectionConfig client_conn_config {
        "127.0.0.1",
        5001,
        Fix::Role::INITIATOR,
        1
    };

    Fix::SessionParameters client_session_params {
        .sender_comp_id = "CLT",
        .target_comp_id = "SRV",
        .heart_beat_int = 30
    };

    Fix::SessionCreationConfig client_config {
        client_conn_config.role,
        client_conn_config,
        client_session_params
    };

    std::vector<Fix::SessionCreationConfig> configs{
        server_config,
        client_config
    };


    Fix::Reactor reactor;
    Fix::AsioConnectionFactory conn_factory{reactor.context()};
    Fix::Cli::FixShell shell;
    ApplicationExample app = Fix::make_application<ApplicationExample>(nullptr, reactor.context());
    Fix::SessionManager session_manager{conn_factory, reactor.context(), app.get_app_sink()};
    app.set_session_manager(&session_manager);


    auto session_ids = session_manager.create_all(configs);
    app.set_session_ids(std::move(session_ids));


    // run the engine
    reactor.run();
    shell.run();


    // cleanup
    session_manager.stop_all();
    reactor.stop();
    reactor.wait();



    
}