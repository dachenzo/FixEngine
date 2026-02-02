#pragma once
#include <concepts>
#include <fix/core/ApplicationEvents.hpp>

namespace Fix {

    struct SessionManager;

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

    struct Application {};

    struct DummyApplication {
        void on_event(InBoundAppEvent&& event) {}
    }; 
}
