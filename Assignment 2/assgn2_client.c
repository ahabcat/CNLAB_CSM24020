#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345
#define BUF_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUF_SIZE];
    char request[BUF_SIZE];

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Server IP = h1’s IP (10.0.0.1 usually in Mininet)
    if (inet_pton(AF_INET, "10.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("invalid address / not supported");
        close(sock);
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connection failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Connected to fruit server.\n");
    printf("Enter request in format: fruit_name quantity\n");

    while (1) {
        printf(">> ");
        fgets(request, sizeof(request), stdin);

        // exit condition
        if (strncmp(request, "exit", 4) == 0) {
            break;
        }

        send(sock, request, strlen(request), 0);

        memset(buffer, 0, BUF_SIZE);
        read(sock, buffer, BUF_SIZE);
        printf("Server response:\n%s\n", buffer);

        // reconnect for new request
        close(sock);
        sock = socket(AF_INET, SOCK_STREAM, 0);
        connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    }

    close(sock);
    return 0;
}
