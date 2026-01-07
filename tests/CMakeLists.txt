
# add_executable(test_session_start
#     test_session_start.cpp
# )

# add_executable(client
#     client.cpp
# )

# add_executable(server
#     server.cpp
# )

# add_executable(test_parser
#     test_parser.cpp
# )

# add_executable(test_log_mpsc
#     test_log_mpsc.cpp
# )

# add_executable(test_log_file_sinks
#     test_log_file_sinks.cpp
# )

# add_executable(test_multi
#     test_multi.cpp
# )

# add_executable(test_registry
#     test_registry.cpp
# )

# add_executable(test_validate_header
#     test_validate_header.cpp
# )

# add_executable(test_repeating_group_validation
#     test_repeating_group_validation.cpp
# )






# target_link_libraries(test_session_start PRIVATE fix_engine)
# target_link_libraries(client PRIVATE fix_engine)
# target_link_libraries(server PRIVATE fix_engine) 
# target_link_libraries(test_parser PRIVATE fix_engine) 
# target_link_libraries(test_log_file_sinks PRIVATE fix_engine) 
# target_link_libraries(test_log_mpsc PRIVATE fix_engine) 
# target_link_libraries(test_repeating_group_validation PRIVATE fix_engine) 
# target_link_libraries(test_validate_header PRIVATE fix_engine) 
# target_link_libraries(test_registry PRIVATE fix_engine) 
# target_link_libraries(test_multi PRIVATE fix_engine) 

add_executable(fix_tests
    test_validator.cpp
    test_tagscratch.cpp
    test_parser.cpp
    test_arena.cpp
    test_factoryscratch.cpp
)


target_link_libraries(fix_tests PRIVATE GTest::gtest_main fix_engine)

include(GoogleTest)
gtest_discover_tests(fix_tests)