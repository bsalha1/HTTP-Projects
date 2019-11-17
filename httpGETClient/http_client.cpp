#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <iostream>

using namespace std;

class http_client
{
    private:
        int port = 80;
        int client_socket;
        char * address;
        char request[4096];
        struct sockaddr_in server_address;

    public:
        http_client(char * address, int port)
        {
            this->address = address;
            this->port = port;

            memset(&server_address, 0, sizeof(server_address));
            server_address.sin_family = AF_INET;
            server_address.sin_port = htons(port);
            server_address.sin_addr.s_addr = inet_addr(address);
        }

        ~http_client()
        {
            if(close(client_socket) == -1)
            {
                fprintf(stderr, "[FAIL] Socket %d failed to close: %s\n", client_socket, strerror(errno));
            }
            fprintf(stderr, "[OK] Socket %d closed.\n", client_socket);
        }


        // Create a socket
        bool createSocket()
        {
            client_socket = socket(AF_INET, SOCK_STREAM, 0);
            if (client_socket == -1)
            {
                fprintf(stderr, "[FAIL] Socket %d failed to open: %s\n", client_socket, strerror(errno));
                return false;
            }
            fprintf(stderr, "[OK] Created socket %d\n", client_socket);
            return true;
        }



        // Connect to the server
        int connectToServer(int socket)
        {
            if (connect(socket, (struct sockaddr *) &server_address, sizeof(server_address)) == -1)
            {
                fprintf(stderr, "[FAIL] Socket %d failed to connect to server %s:%d: %s\n", socket, address, port, strerror(errno));
                return -1;
            }
            fprintf(stderr, "[OK] Socket %d connected to server %s:%d\n", socket, address, port);
            return 0;
        }



        // Send an HTTP request to the server 
        int sendRequest(const int socket, char * command)
        {
            char command_tail[2048] = " / HTTP/1.1\r\n\r\n";
            strcat(command_tail, command);

            if(send(socket, command_tail, sizeof(command_tail), 0) == -1)
            {
                fprintf(stderr, "[FAIL] Failed to send request to server on socket %d\n: %s\n", socket, strerror(errno));
                return -1;
            }
            fprintf(stderr, "[OK] Request sent on socket %d\n", socket);
            return 0;
        }



        // Get response from earlier HTTP request from server
        static int getResponse(const int socket)
        {
            char response[4096];
            fprintf(stderr, "[OK] Getting response to server on socket %d...\n", socket);
            if(recv(socket, &response, sizeof(response), 0) == -1)
            {
                fprintf(stderr, "[FAIL] Failed to receive data via socket %d: %s\n", socket, strerror(errno));
                return -1;
            }
            fprintf(stderr, "[OK] Response from server %s\n", response);
            return 0;
        }



        int getSocket()
        {
            return this->client_socket;
        }



        
};

int main()
{
    http_client client = http_client("128.211.212.44", 80);
    char * request = "GET";
    int socket1;

    client.createSocket();
    socket1 = client.getSocket();
    
    client.connectToServer(socket1);
    client.sendRequest(socket1, request);
    client.getResponse(socket1);

    return EXIT_SUCCESS;
}
