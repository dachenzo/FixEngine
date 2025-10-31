
add_executable(test_session_start
    test_session_start.cpp
)

add_executable(client
    client.cpp
)

add_executable(server
    server.cpp
)

add_executable(test_parser
    test_parser.cpp
)

add_executable(test_log_mpsc
    test_log_mpsc.cpp
)



target_link_libraries(test_session_start PRIVATE fix_engine)
target_link_libraries(client PRIVATE fix_engine)
target_link_libraries(server PRIVATE fix_engine) 
target_link_libraries(test_parser PRIVATE fix_engine) 
target_link_libraries(test_log_mpsc PRIVATE fix_engine) 