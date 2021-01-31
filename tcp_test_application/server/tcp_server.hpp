#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP

#include <string>
#include <netinet/in.h>
#include <poll.h>
#include <vector>
#include <unordered_map>

#define MTU_SIZE 1200

class tcp_server {

    public:

        tcp_server(int port);
       ~tcp_server();

        bool get_init_status();
        void accept_connection();

    private:

        bool init_status;
        int socket_handle{0}; // Zero initialization, server_handle is more meaningful name
        struct sockaddr_in server_address;

        int client_handle{0}; // Zero initialization
        struct sockaddr_in client_address;
        std::string client_ip;
        char data[MTU_SIZE];

        //NOTE: Check for other efficient ds to handle the socket fds
        std::vector<struct pollfd> poll_fds_vec;

        // Store socket handles, port numbers and connection status
        // NOTE: Assuming socket hadles are unique numbers for all active connections
        using PortID = std::string;
        using SokcetHandle = int;
        using ConnectionStatus = bool;
        std::unordered_map<SokcetHandle, std::pair<PortID, ConnectionStatus>> connections_map;
};

#endif // TCP_SERVER_HPP
