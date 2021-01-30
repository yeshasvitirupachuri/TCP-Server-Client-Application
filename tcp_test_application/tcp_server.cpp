#include "tcp_server.hpp"

#include <iostream>
#include <vector>
#include <cstring>

#include <sys/socket.h>
#include <arpa/inet.h>

// Reference : https://pubs.opengroup.org/onlinepubs/9699919799/
// search for socket.h

using namespace std;

//---------------------------------------------------------------------------------------------------------------------
tcp_server::tcp_server(int port) {

    std::cout << "[info] server initialization in progress ... " << std::endl;

    socket_handle = socket(AF_INET, SOCK_STREAM, 0);

    // Check status of server socket fd
    if (socket_handle == 0)
    {
        std::cerr << "[error] server socket initialization failed!" << std::endl;
        return;
    }

    // Socket fd configuration options buffer
    std::vector<int> options = {SO_REUSEPORT, SO_REUSEADDR};

    // setsockopt(socket_handle, SOL_SOCKET, 2, (char*)&options, sizeof(options));
    // Configure socket fd with option SO_REUSEADDR (2)
    if( setsockopt(socket_handle, SOL_SOCKET, options[1], (char*)&options, sizeof(options)) == -1)
    {
        std::cerr << "[error] server socket options configuration failed!" << std::endl;
        return;
    }

    // Set socket server address
    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(port);

    // check for errors
    if( bind(socket_handle, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        std::cerr << "[error] server socket binding failed to port " << port << std::endl;
        return;
    }

    // Check for errors
    // TODO: Change backlog option to have multiple client connections
    if(listen(socket_handle, 1) == -1)
    {
        std::cerr << "[error] server socket at port " << port << " failed to start listening!" << std::endl;
        return;
    }

    // todo: handle errors of this constructor
    std::cout << "[info] server initialization done successfully ... " << std::endl;
    std::cout << "[info] server socket port : " << port << std::endl;
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
