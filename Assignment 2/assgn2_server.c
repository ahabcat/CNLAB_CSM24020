#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <netinet/in.h>

#define PORT 12345
#define BUF_SIZE 1024
#define MAX_CUSTOMERS 100

// Fruit stock structure
typedef struct {
    char name[20];
    int quantity;
    int last_sold;
    char timestamp[64];
} Fruit;

Fruit fruits[] = {
    {"apple", 10, 0, ""},
    {"banana", 8, 0, ""},
    {"mango", 5, 0, ""},
    {"orange", 7, 0, ""}
};
int num_fruits = 4;

// Store unique customers
struct sockaddr_in customers[MAX_CUSTOMERS];
int customer_count = 0;

// Check if customer is new
int is_new_customer(struct sockaddr_in addr) {
    for (int i = 0; i < customer_count; i++) {
        if (customers[i].sin_addr.s_addr == addr.sin_addr.s_addr &&
            customers[i].sin_port == addr.sin_port) {
            return 0; // already exists
        }
    }
    if (customer_count < MAX_CUSTOMERS) {
        customers[customer_count++] = addr;
    }
    return 1; // new customer
}

// Get formatted time string
void get_time_str(char *buf, int size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buf, size, "%Y-%m-%d %H:%M:%S", t);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address, client_addr;
    socklen_t addrlen = sizeof(address);
    char buffer[BUF_SIZE];
    char response[BUF_SIZE];

    // Create TCP socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Define server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(server_fd, 5) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Fruit server listening on port %d...\n", PORT);

    while (1) {
        // Accept client
        addrlen = sizeof(client_addr);
        new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);
        if (new_socket < 0) {
            perror("accept failed");
            continue;
        }

        memset(buffer, 0, BUF_SIZE);
        read(new_socket, buffer, BUF_SIZE);
        printf("Request from client %s:%d -> %s\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buffer);

        // Parse input: fruit_name quantity
        char fruit_name[20];
        int qty;
        if (sscanf(buffer, "%s %d", fruit_name, &qty) != 2) {
            snprintf(response, sizeof(response), "Invalid format. Use: fruit_name quantity");
            send(new_socket, response, strlen(response), 0);
            close(new_socket);
            continue;
        }

        int found = 0;
        for (int i = 0; i < num_fruits; i++) {
            if (strcmp(fruits[i].name, fruit_name) == 0) {
                found = 1;
                if (fruits[i].quantity >= qty) {
                    fruits[i].quantity -= qty;
                    fruits[i].last_sold = qty;
                    get_time_str(fruits[i].timestamp, sizeof(fruits[i].timestamp));

                    // Add customer if new
                    is_new_customer(client_addr);

                    // Build success message
                    snprintf(response, sizeof(response),
                             "Transaction successful!\n"
                             "You bought %d %s(s).\n"
                             "Remaining stock: %d\n"
                             "Last sold: %d at %s\n"
                             "Unique customers so far: %d\n",
                             qty, fruits[i].name, fruits[i].quantity,
                             fruits[i].last_sold, fruits[i].timestamp,
                             customer_count);
                } else {
                    snprintf(response, sizeof(response),
                             "Sorry, only %d %s(s) available.",
                             fruits[i].quantity, fruits[i].name);
                }
                break;
            }
        }

        if (!found) {
            snprintf(response, sizeof(response), "Sorry, %s not available.", fruit_name);
        }

        send(new_socket, response, strlen(response), 0);
        close(new_socket);
    }

    close(server_fd);
    return 0;
}
