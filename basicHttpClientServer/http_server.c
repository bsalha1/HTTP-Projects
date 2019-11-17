#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <errno.h>
#include <signal.h>

#define MAX_SOCKETS 64

void sigHandler(int signal);

int sockets[MAX_SOCKETS] = {0};

int main()
{
    signal(SIGINT, sigHandler);
    char response_data[1024] = {0};
    char http_header[2048] = "HTTP/1.1 200 OK \r\n\n";
    int html_file;
    int server_socket;
    int client_socket;
    int port = 80;
    struct sockaddr_in server_address;

    // Open HTML file and read into application
    html_file = open("pages/index.html", O_RDONLY);
    if(html_file == -1)
    {
        perror("File open fail");
        return EXIT_FAILURE;
    }
    fprintf(stderr, "[OK] index.html opened for reading.\n");

    read(html_file, response_data, sizeof(response_data));
    strcat(http_header, response_data);

    // Create socket .. //
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("Master socket failed to open");
        return EXIT_FAILURE;
    }
    fprintf(stderr, "[OK] Master socket %d opened.\n", server_socket);



    // Define address
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = ((in_addr_t) 0);



    // Bind socket to server ... //
    if (bind(server_socket, (struct sockaddr * ) &server_address, sizeof(server_address)) == -1)
    {
        perror("Failed to bind socket to server");
        return EXIT_FAILURE;
    }



    // Listen ... //
    if (listen(server_socket, 5) == -1)
    {
        perror("Failed to initiate listening");
        return EXIT_FAILURE;
    }
    fprintf(stderr, "[OK] Server is listening...\n");



    // Create client socket and send info
    struct sockaddr_in client_addr;
    socklen_t client_addr_len;
    char * client_address;
    int client_port;
    while(1)
    {
        // Open client socket ... //
        client_socket = accept(server_socket, &client_addr, &client_addr_len);
        if(client_socket == -1)
        {
            fprintf(stderr, "[FAIL] Client socket not accepted: %s\n", strerror(errno));
            return EXIT_FAILURE;
        }
        client_address = inet_ntoa(client_addr.sin_addr);
        client_port = ntohs(client_addr.sin_port);
        fprintf(stderr, "[OK] Client %s:%d socket %d accepted.\n", client_address, client_port, client_socket);



        // Send client socket the header ... //
        if(send(client_socket, http_header, sizeof(http_header), 0) == -1)
        {
            fprintf(stderr, "[FAIL] Data failed to send to client socket %d: %s\n", client_socket, strerror(errno));
            return EXIT_FAILURE;
        }
        fprintf(stderr, "[OK] Data sent to client socket %d.\n", client_socket);



        // Close client socket ... //
        if(close(client_socket) == -1)
        {
            fprintf(stderr, "[FAIL] Client socket %d failed to close: %s\n", client_socket, strerror(errno));
            return EXIT_FAILURE;
        }
        fprintf(stderr, "[OK] Client socket %d closed.\n", client_socket);
    }

    return EXIT_SUCCESS;
}

void sigHandler(int signal)
{
    close(6);
    exit(EXIT_SUCCESS);
}
