#include "tcp_server.hpp"

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

    tcp_server* server = new tcp_server(SERVER_PORT);

    // catch SIGINT to stop the server gracefully
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    while (!term && ((sigaction(SIGINT, &sa, NULL) != -1)) ) {

        // Add polling to repeatedly accept new connections
        // Ref: https://stackoverflow.com/questions/19557941/implementing-poll-on-a-tcp-servers-read-write
        server->accept_connection();
         
    }

    delete server;

    // did we forget something?
    return 0;
}
