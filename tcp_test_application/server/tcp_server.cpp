#include "tcp_server.hpp"

#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>

// Ref: https://pubs.opengroup.org/onlinepubs/9699919799/functions/fcntl.html
#include<fcntl.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

// Reference : https://pubs.opengroup.org/onlinepubs/9699919799/
// search for socket.h

#define POLLING_TIMEOUT_MS 100
#define MAX_CLIENT_CONN 100

using namespace std;

static bool dead_connections = false;

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

                // Ref: https://stackoverflow.com/questions/9604050/so-keepalive-and-poll
                // Ref: https://tldp.org/HOWTO/TCP-Keepalive-HOWTO/usingkeepalive.html
                // Configure client ports to be monitored
                int opt = 1;
                bool config_check = setsockopt(client_handle, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));
                config_check += setsockopt(client_handle, SOL_TCP, TCP_KEEPIDLE, &opt, sizeof(opt));
                config_check += setsockopt(client_handle, SOL_TCP, TCP_KEEPINTVL, &opt, sizeof(opt));
                config_check += setsockopt(client_handle, SOL_TCP, TCP_KEEPCNT, &opt, sizeof(opt));

                if( config_check != 0)
                {
                    std::cerr << "[error] client " <<  ntohs(client_address.sin_port) << " socket options configuration failed!" << std::endl;
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

        if (poll_fds_vec[i].revents & POLLHUP)
        {
            // Close and update the connection handle
            auto it = connections_map.find(poll_fds_vec[i].fd);
            close(poll_fds_vec[i].fd);
            poll_fds_vec[i].fd = -1;

            // Set port connection to false
            it->second.second = false;
            std::cout << "Port " << it->second.first << " hungup ... " << std::endl;

            // Flip dead connections flag on
            dead_connections = true;
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

                // Send reply to client
                std::string ack = "ACK Client [" + it->second.first + "] ";
                send(poll_fds_vec[i].fd, ack.c_str(), strlen(ack.c_str()), MSG_WAITFORONE);
            }
        }
    }

    // Check and drop for inactive connections
    if (poll_fds_vec.size() >= 2 && dead_connections)
    {
        std::cout << "=====  Server Cleanup In Progress  ======" << std::endl;
        // Remove client handles of closed connections from poll fds
        auto it = poll_fds_vec.begin();
        while(it != poll_fds_vec.end())
        {
            if (it->fd != -1)
            {
                it++;
                continue;
            }

            it = poll_fds_vec.erase(it);

        }

        // Remove closed connections from connections map
        auto map_it = connections_map.begin();
        while(map_it != connections_map.end())
        {
            if (map_it->second.second != false)
            {
                map_it++;
                continue;
            }

            std::cout << "[info] clearing connection " << map_it->second.first << std::endl;
            map_it = connections_map.erase(map_it);
        }

        std::cout << "=====  Server Cleanup Done ======" << std::endl;

        // Flip dead connections flag off
        dead_connections = false;
    }

}

//---------------------------------------------------------------------------------------------------------------------
tcp_server::~tcp_server(){

    if (init_status)
    {
        // Close all socket handles
        auto it = poll_fds_vec.end();
        while(it != poll_fds_vec.begin())
        {
            close(it->fd);
            it--;
        }

        // Closing server socket handle
        close(socket_handle);

        std::cout << "[info] server terminated ... " << std::endl;
    }

    // Clear server and client socket handles
    socket_handle = 0;
    client_handle = 0;
}
