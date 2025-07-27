#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "DEV_Config.h"
#include "EPD_7in3f.h"

#define PORT 8080
#define BUFFER_SIZE 1024

// E-paper initialization
void init_epaper() {
    printf("E-paper initialized\n");
    if (DEV_Module_Init() != 0) {
        printf("DEV_Module_Init failed\n");
        return;
    }
    EPD_7IN3F_Init();
}

// Clearing e-paper
void clear_epaper() {
    printf("E-paper cleared\n");
    EPD_7IN3F_Clear(EPD_7IN3F_WHITE); 
}

void handle_request(int client_socket) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_read < 0) {
        perror("recv failed");
        return;
    }
    buffer[bytes_read] = '\0';

    printf("Received request:\n%s\n", buffer);

    // Serve index.html
    if (strstr(buffer, "GET / HTTP/1.1") || strstr(buffer, "GET /index.html HTTP/1.1")) {
        FILE *html_file = fopen("index.html", "r");
        if (html_file == NULL) {
            const char *response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found";
            send(client_socket, response, strlen(response), 0);
        } else {
            fseek(html_file, 0, SEEK_END);
            long file_size = ftell(html_file);
            fseek(html_file, 0, SEEK_SET);

            char *html_content = malloc(file_size + 1);
            fread(html_content, 1, file_size, html_file);
            html_content[file_size] = '\0';
            fclose(html_file);

            char header[BUFFER_SIZE];
            snprintf(header, BUFFER_SIZE, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %ld\r\n\r\n", file_size);
            send(client_socket, header, strlen(header), 0);
            send(client_socket, html_content, file_size, 0);
            free(html_content);
        }
    }
    // Handle clear-epaper POST request
    else if (strstr(buffer, "POST /clear-epaper HTTP/1.1")) {
        clear_epaper();
        const char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nE-paper clear command sent.";
        send(client_socket, response, strlen(response), 0);
    }
    else {
        const char *response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nNot Found";
        send(client_socket, response, strlen(response), 0);
    }

    close(client_socket);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket to port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    // Initialize e-paper (placeholder)
    init_epaper();

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        handle_request(new_socket);
    }

    return 0;
}