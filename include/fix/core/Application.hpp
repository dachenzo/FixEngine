#pragma once
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/any_io_executor.hpp>
#include <concepts>
#include <cstddef>
#include <iostream>
#include <boost/asio/executor.hpp>
#include <fix/core/Message.hpp>
#include <fix/message/MessageList.hpp>
#include <fix/core/SessionManager.hpp>
#include <fix/core/ApplicationEvents.hpp>
#include <utility>

namespace Fix {



    template <typename App, typename ...Args>
    concept ApplicationLike = 
    std::constructible_from<App, SessionManager&, Args...> &&
    requires (App app) {
        { app.get_name() } -> std::convertible_to<std::string>;
        { app.get_app_sink() } -> std::convertible_to<AppSink>; 
    }
    ;


    template <typename App, typename... Args>
    requires ApplicationLike<App, Args...>
    App make_application(SessionManager& session_manager, Args&&... args) {
        return App{session_manager, std::forward<Args>(args)...};
    }

    

    struct DummyApplication {
        
        DummyApplication(SessionManager& session_manager, boost::asio::io_context& io_context):
        exec_{boost::asio::make_strand(io_context.get_executor())},
        session_manager_{session_manager}
        {}
        std::string get_name() { return "DummyApplication"; }
        AppSink get_app_sink() { 
            return [this](InBoundAppEvent&& event) {
                boost::asio::post(exec_, [this, ev = std::move(event)]() mutable {
                    process_event_(std::move(ev));
                });
            }; 
        }

        private:
        void process_event_(InBoundAppEvent&& event) {
            // assume in strand already
            std::cout << "DummyApplication received message of type: " 
                      << *event.valid_message.header_cache_.slots[static_cast<size_t>(CacheSlot::MsgType)] 
                      << " for session: " << event.session_id.id << std::endl;
            GenericMessage<GenericField> msg{{"Yello", 9250}};
            OutBoundAppMsg out_msg{std::move(msg), event.session_id, Fix::Message::Custom::MsgType};
            session_manager_.send(std::move(out_msg));  

        }

        boost::asio::strand<boost::asio::any_io_executor> exec_;
        SessionManager& session_manager_;

    }; 
}
