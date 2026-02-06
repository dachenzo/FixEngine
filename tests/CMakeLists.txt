
add_executable(fix_tests
    test_validator.cpp
    test_tagscratch.cpp
    test_arena.cpp
    test_factoryscratch.cpp
    test_message_factory.cpp
    test_message_store.cpp
    test_linear_buffer.cpp
    test_framer.cpp 
    test_ring.cpp
    test_parser.cpp
    test_recovery_cache.cpp
    test_admin_schemas.cpp
    test_session_pool.cpp
    test_session_e2e.cpp
    test_session_manager.cpp
)

add_executable(shelltest
    test_shell.cpp
)


target_link_libraries(fix_tests PRIVATE GTest::gtest_main fix_engine)
target_link_libraries(shelltest PRIVATE GTest::gtest_main fix_engine)

include(GoogleTest)
gtest_discover_tests(fix_tests)