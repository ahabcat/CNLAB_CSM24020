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
    char buffer[BUF_SIZE] = {0};
    char *hi = "Hi";

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Connect to server (server host has IP 10.0.0.1 in Mininet)
    if (inet_pton(AF_INET, "10.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("invalid address/ address not supported");
        close(sock);
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connection failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Send "Hi" to server
    send(sock, hi, strlen(hi), 0);
    printf("Sent: Hi\n");

    // Receive server response
    read(sock, buffer, BUF_SIZE);
    printf("Server says: %s\n", buffer);

    close(sock);
    return 0;
}
