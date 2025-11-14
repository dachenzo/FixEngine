cmake_minimum_required(VERSION 3.15)
project(fix_engine LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_BUILD_TYPE Debug)

add_subdirectory(src)



add_subdirectory(tests)
# add_subdirectory(examples)


find_package(Boost REQUIRED)

target_link_libraries(fix_engine PUBLIC Boost::boost)


add_executable(fix_main src/main.cpp)
target_link_libraries(fix_main PRIVATE fix_engine)