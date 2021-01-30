#include "tcp_server.hpp"

#include <csignal>

#define SERVER_PORT 6699

static bool terminate = false;

void signal_handler(int sig)
{
    switch (sig) {

        case SIGINT:
            terminate = true;
            break;
        default:
            break;
    }
}

int main(int argc, char** argv){

    tcp_server* server = new tcp_server(SERVER_PORT);

    // catch SIGINT to stop the server gracefully
    signal(SIGINT, signal_handler);

    while (!terminate){

        // todo: fix this function to repeatedly accept new connections
        server->accept_connection();
         
    }

    delete server;

    // did we forget something?

    return 0;
}
