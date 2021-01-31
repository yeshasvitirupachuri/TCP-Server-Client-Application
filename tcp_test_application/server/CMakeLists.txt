set(PROJECT_NAME "tcp_server")

add_executable(${PROJECT_NAME} main.cpp tcp_server.cpp tcp_server.hpp)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
