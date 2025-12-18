#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define BUFFER_SIZE 1024

void receive_file(int sock);
void send_file(int sock);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./server <port>\n");
        return 1;
    }

    int server_fd, client_fd;
    struct sockaddr_in server, client;
    socklen_t c = sizeof(client);
    char option;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[1]));
    server.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr*)&server, sizeof(server));
    listen(server_fd, 1);

    printf("Server waiting...\n");
    client_fd = accept(server_fd, (struct sockaddr*)&client, &c);
    printf("Client connected.\n");

    while (1) {
        recv(client_fd, &option, 1, 0);

        if (option == 'D') {     // Client wants to download file
            send_file(client_fd);
        }
        else if (option == 'U') { // Client will upload file
            receive_file(client_fd);
        }
        else if (option == 'Q') {
            break;
        }
    }

    close(client_fd);
    close(server_fd);
    return 0;
}

void send_file(int sock) {
    char filename[256];
    char buffer[BUFFER_SIZE];
    FILE *fp;

    recv(sock, filename, 256, 0);
    printf("Client requests download: %s\n", filename);

    fp = fopen(filename, "rb");
    if (!fp) {
        strcpy(buffer, "FILE_NOT_FOUND");
        send(sock, buffer, strlen(buffer), 0);
        return;
    }

    time_t start = time(NULL);

    int bytes;
    while ((bytes = fread(buffer, 1, BUFFER_SIZE, fp)) > 0) {
        send(sock, buffer, bytes, 0);
    }

    time_t end = time(NULL);
    printf("Download time (server): %ld seconds\n", end - start);

    fclose(fp);
}

void receive_file(int sock) {
    char filename[256];
    char buffer[BUFFER_SIZE];
    FILE *fp;

    recv(sock, filename, 256, 0);
    printf("Receiving upload: %s\n", filename);

    fp = fopen(filename, "wb");

    time_t start = time(NULL);

    int bytes;
    while ((bytes = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        if (bytes < BUFFER_SIZE)
            fwrite(buffer, 1, bytes, fp);
        else
            fwrite(buffer, 1, BUFFER_SIZE, fp);

        if (bytes < BUFFER_SIZE)
            break;
    }

    time_t end = time(NULL);
    printf("Upload time (server): %ld seconds\n", end - start);

    fclose(fp);
}

