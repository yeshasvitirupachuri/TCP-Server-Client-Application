#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP

#include <string>
#include <netinet/in.h>

#define MTU_SIZE 1200

class tcp_server {

    public:
        tcp_server(int port);
       ~tcp_server();
        
        void accept_connection();


    private:
        int socket_handle{0};
        struct sockaddr_in server_address;

        int client_handle{0};
        struct sockaddr_in client_address;
        std::string client_ip;
        char data[MTU_SIZE];

};







#endif // TCP_SERVER_HPP
