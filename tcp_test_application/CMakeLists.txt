cmake_minimum_required(VERSION 3.12)

project(tcp_test_application LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(server)
add_subdirectory(client)
