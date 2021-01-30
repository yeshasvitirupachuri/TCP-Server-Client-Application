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

enum class ClientMode
{
    SINGLE_PING = 0,
    USER_INPUT,
};

static void sendMsg(int& socket, std::string& msg)
{
    int size = 0;
    while ( size < strlen(msg.c_str()) ){

        // On successfull send, the function returns number of bytes sendgetline(cin, userMsg);
        // NOTE: Double check if the flag needs to be MSG_WAITFORONE
        size = send(socket, msg.c_str(), strlen(msg.c_str()), MSG_WAITFORONE);
    }

}

// Reference: https://www.geeksforgeeks.org/socket-programming-cc/
// Used fo cleanup of client application

int main(int argc, char *argv[]){

    // Ref: https://stackoverflow.com/questions/19140892/strange-sigaction-and-getline-interaction
    // Ref: https://www.gnu.org/software/libc/manual/html_node/Sigaction-Function-Example.html
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    // Check for server port number from input arguments
    int server_port = SERVER_PORT;
    if (argc < 2)
    {
        std::cout << "[info] using default server port " << server_port << std::endl;
    }
    else
    {
        server_port = atoi(argv[1]);
        std::cout << "[info] using server port " << server_port << std::endl;
    }

    // Check for client mode from input arguments, assuming succeded by port number argument
    ClientMode client_mode = ClientMode::SINGLE_PING;

    if (argc == 3 && atoi(argv[2]) == 1)
    {
        client_mode = ClientMode::USER_INPUT;
        std::cout << "[info] using client in user input mode" << std::endl;
    }
    else
    {
        std::cout << "[info] using client in single ping message mode" << std::endl;
    }

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
    server_address.sin_port = htons(server_port);

    // Connect to server, and send messages
    if(connect(client_handle, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        std::cerr << "[error] failed to connect client port to server port " << server_port << std::endl;
        return -1;
    }

    std::string clientMsg;
    switch (client_mode) {
        case ClientMode::SINGLE_PING:
        {
            clientMsg = "Client message : Hello server ... ";
            sendMsg(client_handle, clientMsg);
            break;
        }
        case ClientMode::USER_INPUT:
        {
            std::string userMsg;
            while(!term && (sigaction(SIGINT, &sa, NULL) != -1))
            {
                userMsg.clear();
                clientMsg.clear();
                cout << "Write message to server : ";

                getline(cin, userMsg);

                if (userMsg.size() != 0)
                {
                    clientMsg = "Client message : " + userMsg;
                }

                sendMsg(client_handle, clientMsg);
        }
        break;
    }
    default:
        break;
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
