#include <iostream>
#include <signal.h>
#include <memory>
#include <vector>
#include <cstring>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define CLIENTS 1
#define CLIENT_PORT_START_INDEX 12000
#define SERVER_PORT 6699
#define EOL_STRING "\n"

using namespace std;

static bool term = false;

static void signal_handler(int sig)
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

    // Ref: https://stackoverflow.com/questions/19140892/strange-sigaction-and-getline-interaction
    // Ref: https://www.gnu.org/software/libc/manual/html_node/Sigaction-Function-Example.html
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

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
        std::cerr << "[error] failed to connect client port to server port " << SERVER_PORT << std::endl;
        return -1;
    }

    std::string userMsg;
    std::string clientMsg;
    while(!term && (sigaction(SIGINT, &sa, NULL) != -1))
    {
        userMsg.clear();
        cout << "Write message to server : ";

        getline(cin, userMsg);

        if (userMsg.size() != 0)
        {
            clientMsg = "Client message : " + userMsg;
        }

        int size = 0;
        while ( size < strlen(clientMsg.c_str()) ){

            // On successfull send, the function returns number of bytes sendgetline(cin, userMsg);
            // NOTE: Double check if the flag needs to be MSG_WAITFORONE
            size = send(client_handle, clientMsg.c_str(), strlen(clientMsg.c_str()), MSG_WAITFORONE);
        }
    }

    // Shutdown client
    while(client_handle != 0 && ( shutdown(client_handle, SHUT_RDWR) == -1) )
    {
        std::cout << "[info] shutting down client ... " << std::endl;
    }

    // Close client socket handle
    close(client_handle);

    std::cout << "[info] client terminated ... " << std::endl;

    return 0;
}
