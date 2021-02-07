#include "tcp_server.hpp"

#include <iostream>
#include <signal.h>

#define SERVER_PORT 6699

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

int main(int argc, char** argv){

    // catch SIGINT to stop the server gracefully
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    try
    {
        // Initialize server instance
        tcp_server::ptr server(new tcp_server(SERVER_PORT));

        while (server != nullptr && !term && ((sigaction(SIGINT, &sa, NULL) != -1)) ) {

            // Add polling to repeatedly accept new connections
            // Ref: https://stackoverflow.com/questions/19557941/implementing-poll-on-a-tcp-servers-read-write
            server->accept_connection();

        }

    }  catch (std::exception& e) {
        std::cout << "[error] " << e.what() << std::endl;
    }

    return 0;
}
