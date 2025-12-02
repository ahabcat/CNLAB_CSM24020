#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345
#define BUF_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in serv_addr;
    char buffer[BUF_SIZE];
    char request[BUF_SIZE];
    socklen_t addr_len = sizeof(serv_addr);

    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "10.0.0.1", &serv_addr.sin_addr); // server IP (h1)

    printf("Connected to UDP fruit server.\n");
    printf("Enter request in format: fruit_name quantity\n");

    while (1) {
        printf(">> ");
        fgets(request, sizeof(request), stdin);
        request[strcspn(request, "\n")] = '\0';

        if (strncmp(request, "exit", 4) == 0)
            break;

        sendto(sockfd, request, strlen(request), 0, (struct sockaddr *)&serv_addr, addr_len);

        memset(buffer, 0, BUF_SIZE);
        recvfrom(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *)&serv_addr, &addr_len);

        printf("Server response:\n%s\n", buffer);
    }

    close(sockfd);
    return 0;
}

