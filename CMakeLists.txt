cmake_minimum_required(VERSION 3.15)
project(fix_engine LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_BUILD_TYPE Debug)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_subdirectory(src)



add_subdirectory(tests)
add_subdirectory(examples)


find_package(Boost REQUIRED)

target_link_libraries(fix_engine PUBLIC Boost::boost)

