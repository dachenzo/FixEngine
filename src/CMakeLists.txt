add_library(fix_engine
    
    AsioTimer.cpp
    Clock.cpp
    Config.cpp
    ConnectionAsio.cpp
    Message.cpp
    MessageFactory.cpp
    MessageStore.cpp
    Parser.cpp
    Reactor.cpp
    SeqProvider.cpp
    Serializer.cpp
    Session.cpp
    SessionManager.cpp
    SessionPool.cpp
    utils.cpp
    
)


target_include_directories(fix_engine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
find_package(Boost REQUIRED COMPONENTS system)
target_link_libraries(fix_engine PUBLIC Boost::system)
