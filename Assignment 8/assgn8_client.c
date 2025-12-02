#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int sock;

/* Thread to continuously receive messages */
void *receive_handler(void *unused) {
    char msg[BUFFER_SIZE];

    while (1) {
        memset(msg, 0, BUFFER_SIZE);
        if (recv(sock, msg, BUFFER_SIZE, 0) > 0) {
            printf("%s", msg);
        }
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: ./client <server_ip> <port>\n");
        return 1;
    }

    struct sockaddr_in server;
    pthread_t recv_thread;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));
    server.sin_addr.s_addr = inet_addr(argv[1]);

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("Connect failed");
        return 1;
    }

    printf("Connected to chat. Start typing...\n");

    pthread_create(&recv_thread, NULL, receive_handler, NULL);
    pthread_detach(recv_thread);

    char msg[BUFFER_SIZE];
    while (fgets(msg, BUFFER_SIZE, stdin) != NULL) {
        send(sock, msg, strlen(msg), 0);
        printf("Sent: %s\n", msg);

    }

    close(sock);
    return 0;
}

