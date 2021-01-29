#include <tcp_server.hpp>

#include <iostream>
#include <vector>
#include <cstring>

#include <sys/socket.h>
#include <arpa/inet.h>

//---------------------------------------------------------------------------------------------------------------------
tcp_server::tcp_server(int port) {

    std::cout << "[info] server init" << std::endl;

    socket_handle = socket(AF_INET, SOCK_STREAM, 0);

    std::vector<int> options = {SO_REUSEPORT, SO_REUSEADDR};

    // todo: check for errors, report them
    setsockopt(socket_handle, SOL_SOCKET, 2, (char*)&options, sizeof(options));

    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(port);

    // todo: check for errors
    bind(socket_handle, (struct sockaddr *)&server_address, sizeof(server_address));

    // todo: check for errors
    listen(socket_handle, 1);

    // todo: handle errors of this constructor
}

//---------------------------------------------------------------------------------------------------------------------
void tcp_server::accept_connection() {
    
    socklen_t client_socket_size = sizeof(client_address);
    client_handle = accept(socket_handle, (struct sockaddr*)&client_address, &client_socket_size); 
    client_ip = inet_ntoa(client_address.sin_addr);

    std::cout << "[info] accepted connection from: " << client_ip << std::endl;

    // todo: check for dead connection and go back to listening for new ones
    while (true){
        int status = recv(client_handle, data, MTU_SIZE, 0);
        if (status != -1 && status != 0){
            std::cout << std::string(data) << std::endl;
        }
    }

}

//---------------------------------------------------------------------------------------------------------------------
tcp_server::~tcp_server(){
    // todo: clean up
}
