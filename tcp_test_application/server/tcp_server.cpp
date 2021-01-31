#include "tcp_server.hpp"

#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>

// Ref: https://pubs.opengroup.org/onlinepubs/9699919799/functions/fcntl.html
#include<fcntl.h>

#include <sys/socket.h>
#include <arpa/inet.h>

// Reference : https://pubs.opengroup.org/onlinepubs/9699919799/
// search for socket.h

#define POLLING_TIMEOUT_MS 100
#define MAX_CLIENT_CONN 100

using namespace std;

bool tcp_server::get_init_status()
{
    return this->init_status;
}

//---------------------------------------------------------------------------------------------------------------------
tcp_server::tcp_server(int port) {

    std::cout << "[info] server initialization in progress ... " << std::endl;

    // Set initialization default status
    init_status = true;

    socket_handle = socket(AF_INET, SOCK_STREAM, 0);

    // Check status of server socket fd
    if (socket_handle == 0)
    {
        std::cerr << "[error] server socket initialization failed!" << std::endl;
        init_status = false;
        return;
    }

    // Socket fd configuration options buffer
    std::vector<int> options = {SO_REUSEPORT, SO_REUSEADDR};

    // setsockopt(socket_handle, SOL_SOCKET, 2, (char*)&options, sizeof(options));
    // Configure socket fd with option SO_REUSEADDR (2)
    if( setsockopt(socket_handle, SOL_SOCKET, options[1], (char*)&options, sizeof(options)) == -1)
    {
        std::cerr << "[error] server socket options configuration failed!" << std::endl;
        init_status = false;
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
        init_status = false;
        return;
    }

    // Change backlog option to have multiple client connections
    if(listen(socket_handle, MAX_CLIENT_CONN) == -1)
    {
        std::cerr << "[error] server socket at port " << port << " failed to start listening!" << std::endl;
        init_status = false;
        return;
    }

    // Initialize poll fd with server socket
    struct pollfd poll_fd{socket_handle, POLLIN, 0};
    poll_fds_vec.push_back(std::move(poll_fd));

    // Add server connection details to map
    PortID portID = strcat(inet_ntoa(server_address.sin_addr), ":") + std::to_string(ntohs(server_address.sin_port));
    connections_map[socket_handle] = std::make_pair<PortID, ConnectionStatus>(std::move(portID), true);

    std::cout << "[info] server initialization done successfully ... " << std::endl;
    std::cout << "[info] server socket port : " << port << std::endl;
}

//---------------------------------------------------------------------------------------------------------------------
void tcp_server::accept_connection() {

    int n = poll(poll_fds_vec.data(), poll_fds_vec.size(), POLLING_TIMEOUT_MS);

    //TODO: Check and drop for inactive connections
    for (int i = 0; i < poll_fds_vec.size(); i++) {

        // Lister to server socket for new connections
        if (i == 0) {
            if (poll_fds_vec[0].revents & POLLIN)
            {
                // Initialize client socket fd from client address
                socklen_t client_socket_size = sizeof(client_address);

                // Blocking call of accept() when connections are empty and O_NONBLOCK (default : false)
                client_handle = accept(socket_handle, (struct sockaddr*)&client_address, &client_socket_size);

                // Check the client socket fd initialization for the connected client address
                if(client_handle == -1)
                {
                    std::cerr << "[error] client socket initialization failed for client address port " << client_address.sin_port << std::endl;
                    return;
                }

                // Initialize poll fd with client socket
                struct pollfd poll_fd{client_handle, POLLIN, 0};
                poll_fds_vec.push_back(std::move(poll_fd));

                client_ip = inet_ntoa(client_address.sin_addr);

                std::cout << "[info] accepted connection from: " << client_ip << " port " << ntohs(client_address.sin_port) << std::endl;

                // Add client details to map
                if(connections_map.find(client_handle) == connections_map.end())
                {

                    PortID portID = strcat(inet_ntoa(client_address.sin_addr), ":") + std::to_string(ntohs(client_address.sin_port));
                    connections_map[client_handle] = std::make_pair<PortID, ConnectionStatus>(std::move(portID), true);
                    auto it = connections_map.find(client_handle);
                }
            }
        }

        // Listen to client sockets for incoming data
        else if (poll_fds_vec[i].revents & POLLIN)
        {
            // Clear data buffer
            memset(data, 0, sizeof(data));

            int size = recv(poll_fds_vec[i].fd, data, MTU_SIZE, 0);

            // Process the incoming data on successful read
            if (size != -1 && size != 0){
                auto it = connections_map.find(poll_fds_vec[i].fd);
                std::cout << "[" << it->second.first << "] "<< std::string(data) << std::endl;
            }
        }
    }

}

//---------------------------------------------------------------------------------------------------------------------
tcp_server::~tcp_server(){

    if (init_status)
    {
        // Shutdown server
        while(socket_handle != 0 && shutdown(socket_handle, SHUT_RDWR) < 0)
        {
            std::cout << "[info] shutting down server ... " << std::endl;
        }

        // Closing server socket handle
        close(socket_handle);

        std::cout << "[info] server terminated ... " << std::endl;
    }

    // Clear server and client socket handles
    socket_handle = 0;
    client_handle = 0;
}
