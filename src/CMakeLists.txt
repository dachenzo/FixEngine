add_library(fix_engine
    ConnectionAsio.cpp
    AsioTimer.cpp
    Session.cpp
    SessionManager.cpp
    Parser.cpp
    Serializer.cpp
    MessageStore.cpp
    Config.cpp
    SessionPool.cpp
    Reactor.cpp
    Message.cpp
)


target_include_directories(fix_engine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
find_package(Boost REQUIRED COMPONENTS system)
target_link_libraries(fix_engine PUBLIC Boost::system)
