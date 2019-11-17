#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>

int main(int argc, char **argv)
{
    // Local Declarations //
    int port = 80;
    int client_socket;
    char * address;
    char * request = "GET / HTTP/1.1\r\n\r\n";
    char response[4096];
    struct sockaddr_in server_address;



    // Handle arguments ... //
    if (argc == 1)
    {
        fprintf(stderr, "Client: please specify an address to request from with the first argument.\n");
        return EXIT_FAILURE;
    }
    else if (argc > 2)
    {
        fprintf(stderr, "Client: too many arguments, only 1 required.\n");
        return EXIT_FAILURE;
    }

    

    // Define socket ... //
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        printf("Client: socket %d failed to open.\n");
        return EXIT_FAILURE;
    }
    printf("Client: socket %d opened.\n", client_socket);



    // Define Address ... //
    address = argv[1];
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    inet_aton(address, &server_address.sin_addr.s_addr);



    // Connect to server...
    if (connect(client_socket, (struct sockaddr *) &server_address, sizeof(server_address)) == -1)
    {
        fprintf(stderr, "Client: failed to connect to %s:%d\n", address, port);
        return EXIT_FAILURE;
    }
    fprintf(stdout, "Client: connected to %s:%d\n", address, port);



    // Send request to server ... //
    if(send(client_socket, request, sizeof(request), 0) == -1)
    {
        fprintf(stderr, "Client: failed to send request to %s:%d\n", address, port);
        return EXIT_FAILURE;
    }
    fprintf(stdout, "Client: sent request to %s:%d\n", address, port);



    // Receive data from server ... //
    if(recv(client_socket, &response, sizeof(response), 0) == -1)
    {
        fprintf(stderr, "Client: failed to receive response from %s:%d\n", address, port);
        return EXIT_FAILURE;
    }
    fprintf(stdout, "Client: received data from %s:%d\n", address, port);

    fprintf(stdout, "Client: response from server %s\n", response);

    close(client_socket);

    return EXIT_SUCCESS;
}