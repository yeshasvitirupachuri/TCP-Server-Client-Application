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

static bool term = false;

void signal_handler(int sig)
{
    switch (sig) {

        case SIGINT:
            term = true;
            break;
        default:
            break;
    }
}

// Reference: https://www.geeksforgeeks.org/socket-programming-cc/
// Used fo cleanup of client application

int main(int argc, char** argv){

    // TODO: Handle server and client port numbes from input arguments

    int client_handle = socket(AF_INET, SOCK_STREAM, 0);

    // NOTE: Check if the client sockets also need to be non-blocking
    if(client_handle == 0)
    {
        std::cerr << "[error] client socket initialization failed!" << std::endl;
        return -1;
    }

    // Prepare server address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(SERVER_PORT);

    // Connect to server, and send messages
    if(connect(client_handle, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        std::cerr << "[warning] failed to connect client port to server port " << SERVER_PORT << std::endl;
    }

    std::string userMsg;
    while(!term)
    {
        userMsg.clear();
        cout << "Write message to server : ";
        std::cin >> userMsg;
        std::string msg = "Client message : " + userMsg;

        int size = 0;
        while ( size < strlen(msg.c_str()) ){

            // On successfull send, the function returns number of bytes send
            // NOTE: Double check if the flag needs to be MSG_WAITFORONE
            size = send(client_handle, msg.c_str(), strlen(msg.c_str()), MSG_WAITFORONE);
        }
    }

    return 0;
}
