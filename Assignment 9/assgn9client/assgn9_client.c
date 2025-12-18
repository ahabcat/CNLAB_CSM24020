#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define BUFFER_SIZE 1024

void download_file(int sock);
void upload_file(int sock);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: ./client <server_ip> <port>\n");
        return 1;
    }

    int sock;
    struct sockaddr_in server;
    char option;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));
    server.sin_addr.s_addr = inet_addr(argv[1]);

    connect(sock, (struct sockaddr*)&server, sizeof(server));
    printf("Connected to server.\n");

    // First: Download
    option = 'D';
    send(sock, &option, 1, 0);
    download_file(sock);

    // Second: Upload
    option = 'U';
    send(sock, &option, 1, 0);
    upload_file(sock);

    // Quit
    option = 'Q';
    send(sock, &option, 1, 0);

    close(sock);
    return 0;
}

void download_file(int sock) {
    char filename[256];
    char buffer[BUFFER_SIZE];

    printf("Enter filename to download from server: ");
    scanf("%s", filename);

    send(sock, filename, 256, 0);

    FILE *fp = fopen(filename, "wb");

    time_t start = time(NULL);

    int bytes;
    while ((bytes = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        if (strcmp(buffer, "FILE_NOT_FOUND") == 0) {
            printf("File not found on server.\n");
            fclose(fp);
            return;
        }
        fwrite(buffer, 1, bytes, fp);
        if (bytes < BUFFER_SIZE)
            break;
    }

    time_t end = time(NULL);
    printf("Download time (client): %ld seconds\n", end - start);

    fclose(fp);
}

void upload_file(int sock) {
    char filename[256];
    char buffer[BUFFER_SIZE];

    printf("Enter filename to upload to server: ");
    scanf("%s", filename);

    send(sock, filename, 256, 0);

    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        printf("File not found in client directory.\n");
        return;
    }

    time_t start = time(NULL);

    int bytes;
    while ((bytes = fread(buffer, 1, BUFFER_SIZE, fp)) > 0) {
        send(sock, buffer, bytes, 0);
    }

    time_t end = time(NULL);
    printf("Upload time (client): %ld seconds\n", end - start);

    fclose(fp);
}

