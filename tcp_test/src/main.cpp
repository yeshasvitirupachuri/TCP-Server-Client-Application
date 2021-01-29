#include <tcp_server.hpp>



int main(int argc, char** argv){


    tcp_server* server = new tcp_server(6699);

    // todo: catch SIGINT to stop the server gracefully

    while (true){

        // todo: fix this function to repeatedly accept new connections
        server->accept_connection(); 
         
    }

    // did we forget something?

    return 0;
}
