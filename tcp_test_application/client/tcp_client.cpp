#include <iostream>
#include <csignal>
#include <memory>
#include <vector>
#include <cstring>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define CLIENTS 1
#define CLIENT_PORT_START_INDEX 12000
#define SERVER_PORT 6699
#define EOL_STRING "\n"

using namespace std;

// Vector of client socket handles
std::vector<std::pair<int, sockaddr_in>> clients;

int main(int argc, char** argv){

    // TODO: Handle server and client port numbes from input arguments

    // Initialize clients
    clients.empty();
    while(clients.size() < CLIENTS)
    {
        int client_handle = 0;
        client_handle = socket(AF_INET, SOCK_STREAM, 0);

        // NOTE: Check if the client sockets also need to be non-blocking
        if(client_handle == 0)
        {
            std::cerr << "[error] client socket initialization failed!" << std::endl;
            return -1;
        }

        // Socket fd configuration options buffer
        std::vector<int> options = {SO_REUSEPORT, SO_REUSEADDR};

        // setsockopt(socket_handle, SOL_SOCKET, 2, (char*)&options, sizeof(options));
        // Configure socket fd with option SO_REUSEADDR (2)
        if( setsockopt(client_handle, SOL_SOCKET, options[1], (char*)&options, sizeof(options)) == -1)
        {
            std::cerr << "[error] client socket options configuration failed!" << std::endl;
            return -1;
        }

        // Set socket server address
        struct sockaddr_in client_address;
        memset(&client_address, 0, sizeof(client_address));

        client_address.sin_family = AF_INET;
        client_address.sin_addr.s_addr = htonl(INADDR_ANY);

        // Assigning port numbers starting from CLIENT_PORT_START_INDEX
        // NOTE: The port numbers are being different from the start index
        client_address.sin_port = htons(CLIENT_PORT_START_INDEX + clients.size());

        // check for errors
        if( bind(client_handle, (struct sockaddr *)&client_address, sizeof(client_address)) == -1)
        {
            std::cerr << "[error] client socket binding failed to port " << client_address.sin_port << std::endl;
            return -1;
        }

        std::cout << "Client address : " << inet_ntoa(client_address.sin_addr) << std::endl;

        clients.push_back(std::pair<int, sockaddr_in>(client_handle, client_address));
    }

    // Prepare server address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(SERVER_PORT);

    // Connect to server, and send messages
    for(auto& c : clients)
    {
        if(connect(c.first, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
        {
            std::cerr << "[warning] failed to connect client port " << inet_ntoa(c.second.sin_addr) << " to server port " << SERVER_PORT << std::endl;
        }
        else
        {
            std::string msg = "pinging server from client at port " + (string)inet_ntoa(c.second.sin_addr) + EOL_STRING;

            int size = 0;
            while ( size < strlen(msg.c_str()) ){

                // On successfull send, the function returns number of bytes send
                // NOTE: Double check if the flag needs to be MSG_WAITFORONE
                size = send(c.first, msg.c_str(), strlen(msg.c_str()), MSG_WAITFORONE);
            }

        }
    }

    std::cout << "[info] clients communication done " << std::endl;

    return 0;
}
