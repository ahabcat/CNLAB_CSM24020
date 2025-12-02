#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 4000
#define BUF_SIZE 1024

int main() {
    int sockfd;
    char expr[BUF_SIZE], buffer[BUF_SIZE];
    struct sockaddr_in serv_addr;
    socklen_t addr_len = sizeof(serv_addr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "10.0.0.1", &serv_addr.sin_addr);   // server host (h1 usually)

    printf("UDP Scientific Calculator Client Started\n");
    printf("Enter expressions like:\n");
    printf(" sin 30\n 10 + 20\n sqrt 9\n inv 5\n");
    printf("Type exit to quit.\n\n");

    while (1) {
        printf(">> ");
        fgets(expr, BUF_SIZE, stdin);
        expr[strcspn(expr, "\n")] = '\0';

        if (strcmp(expr, "exit") == 0) break;

        // ------- send to server -------
        sendto(sockfd, expr, strlen(expr), 0, (struct sockaddr *)&serv_addr, addr_len);

        // ------- graceful UDP reception -------
        memset(buffer, 0, BUF_SIZE);

        // Set 1-second timeout to detect packet loss
        struct timeval tv = {1, 0};
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        int n = recvfrom(sockfd, buffer, BUF_SIZE, 0, NULL, NULL);

        if (n < 0) {
            printf("Timeout: Packet lost or no response from server.\n");
            continue;
        }

        printf("Server: %s\n", buffer);
    }

    close(sockfd);
    return 0;
}

