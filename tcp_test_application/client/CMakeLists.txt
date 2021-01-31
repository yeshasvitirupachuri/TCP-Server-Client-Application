set(PROJECT_NAME "tcp_client")

add_executable(${PROJECT_NAME} tcp_client.cpp)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/test/multi_client_test.sh DESTINATION ${CMAKE_INSTALL_PREFIX}/test)
