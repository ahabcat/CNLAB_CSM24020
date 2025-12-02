#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024

int clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Add client to list */
void add_client(int socket) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] == 0) {
            clients[i] = socket;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

/* Remove client */
void remove_client(int socket) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] == socket) {
            clients[i] = 0;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

/* Broadcast message to all clients */
void broadcast_message(int sender, char *msg) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != 0 && clients[i] != sender) {
            send(clients[i], msg, strlen(msg), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

/* Log messages to log.txt with timestamp */
void log_message(const char *msg) {
    pthread_mutex_lock(&log_mutex);

    FILE *fp = fopen("log.txt", "a");
    if (!fp) {
        perror("Log file error");
        pthread_mutex_unlock(&log_mutex);
        return;
    }

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    fprintf(fp, "[%02d:%02d:%02d] %s",
            t->tm_hour, t->tm_min, t->tm_sec, msg);

    fclose(fp);
    pthread_mutex_unlock(&log_mutex);
}

/* Thread to handle client */
void *client_handler(void *socket_desc) {
    int sock = *(int*)socket_desc;
    char msg[BUFFER_SIZE];

    add_client(sock);

    while (1) {
        memset(msg, 0, BUFFER_SIZE);
        int read_size = recv(sock, msg, BUFFER_SIZE - 1, 0);
        msg[read_size] = '\0';

        if (read_size <= 0) {
            printf("Client disconnected.\n");
            remove_client(sock);
            close(sock);
            pthread_exit(NULL);
        }

        msg[strcspn(msg, "\n")] = '\0';
        printf("Received from client (%d bytes): '%s'\n", read_size, msg);

        // Log and broadcast
        log_message(msg);
        broadcast_message(sock, msg);
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./server <port>\n");
        return 1;
    }

    int server_socket, client_socket, c;
    struct sockaddr_in server, client;
    pthread_t thread_id;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (!server_socket) {
        perror("Could not create socket");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(atoi(argv[1]));

    if (bind(server_socket, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("Bind failed");
        return 1;
    }

    listen(server_socket, 10);

    printf("Server started. Waiting for connections...\n");
    c = sizeof(struct sockaddr_in);

    while ((client_socket = accept(server_socket, (struct sockaddr *)&client, (socklen_t*)&c))) {
        printf("Client connected.\n");

        if (pthread_create(&thread_id, NULL, client_handler, (void*)&client_socket) < 0) {
            perror("Thread creation failed");
            return 1;
        }

        pthread_detach(thread_id);
    }

    return 0;
}

