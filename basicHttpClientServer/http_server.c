#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <errno.h>
#include <signal.h>
#define MAX_SOCKETS 64

void sigHandler(int signal);

int sockets[MAX_SOCKETS] = {0};

int main(int argc, char * argv[])
{
    if(argc != 3)
    {
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, sigHandler);
    char response_data[1024] = {0};
    char http_header[2048] = "HTTP/1.1 200 OK \r\n\n";
    int html_file;
    int server_socket;
    int client_socket;
    char filename[] = "pages/test.html";
    struct sockaddr_in server_address;

    // Open HTML file and read into application
    html_file = open(filename, O_RDONLY);
    if(html_file == -1)
    {
        perror("File open fail");
        return EXIT_FAILURE;
    }
    fprintf(stderr, "[OK] %s opened for reading.\n", filename);

    read(html_file, response_data, sizeof(response_data));
    strcat(http_header, response_data);

    // Create socket .. //
    int optVal = 1;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("Master socket failed to open");
        return EXIT_FAILURE;
    }
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(int));
    fprintf(stderr, "[OK] Master socket %d opened.\n", server_socket);



    // Define address
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[2]));
    server_address.sin_addr.s_addr = inet_addr(argv[1]);



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
    size_t size_sent;

    char image_header[] = 
        "HTTP/1.1 200 OK\r\n\n";
    char imagename[] = "pages/image.jpg";
    char buffer[2048];
    struct stat image_stats;
    int image_file;

    while(1)
    {
        // Open client socket ... //
        client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &client_addr_len);
        if(client_socket == -1)
        {
            fprintf(stderr, "[FAIL] Client socket not accepted: %s\n", strerror(errno));
            return EXIT_FAILURE;
        }
        client_address = inet_ntoa(client_addr.sin_addr);
        client_port = ntohs(client_addr.sin_port);
        fprintf(stderr, "[OK] Client %s:%d socket %d accepted.\n", client_address, client_port, client_socket);

        if(!fork())
        {
            close(server_socket);
            memset(buffer, 0, 2048);
            read(client_socket, buffer, 2047);

            printf("Request: %s\n", buffer);

            if(!strncmp(buffer, "GET /image.jpg", 7))
            {
                fprintf(stderr, "[OK] Sending client an image\n");
                write(client_socket, image_header, sizeof(image_header) - 1); // Send header

                image_file = open(imagename, O_RDONLY);
                if(stat(imagename, &image_stats) == -1)
                {
                    perror("Failed to get file size");
                    exit(EXIT_FAILURE);
                }

                size_sent = sendfile(client_socket, image_file, NULL, image_stats.st_size); // send file
                if(size_sent == -1)
                {
                    perror("Failed to send file to client");
                    exit(EXIT_FAILURE);
                }
                fprintf(stderr, "[OK] Sent %ld bytes\n", size_sent);

                close(image_file);
            }
            else
            {
                write(client_socket, http_header, sizeof(http_header) - 1);
            }
            close(client_socket);
            fprintf(stderr, "[OK] Closed connection with client socket %d\n", client_socket);
            exit(0);
        }
        
        close(client_socket);
    }

    return EXIT_SUCCESS;
}

void sigHandler(int signal)
{
    close(6);

    exit(EXIT_SUCCESS);
}
