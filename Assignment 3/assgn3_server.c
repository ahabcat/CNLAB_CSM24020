#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 12345
#define BUF_SIZE 1024
#define MAX_CUSTOMERS 100

// Structure to store fruit info
typedef struct {
    char name[20];
    int quantity;
    int last_sold;
    char timestamp[64];
} Fruit;

// Initial fruit stock
Fruit fruits[] = {
    {"apple", 10, 0, ""},
    {"banana", 8, 0, ""},
    {"mango", 5, 0, ""},
    {"orange", 7, 0, ""}
};
int num_fruits = 4;

// To store unique customers
struct sockaddr_in customers[MAX_CUSTOMERS];
int customer_count = 0;

// Check if customer is new
int is_new_customer(struct sockaddr_in addr) {
    for (int i = 0; i < customer_count; i++) {
        if (customers[i].sin_addr.s_addr == addr.sin_addr.s_addr &&
            customers[i].sin_port == addr.sin_port) {
            return 0; // Already exists
        }
    }
    if (customer_count < MAX_CUSTOMERS) {
        customers[customer_count++] = addr;
    }
    return 1; // New customer added
}

// Get timestamp string
void get_time_str(char *buf, int size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buf, size, "%Y-%m-%d %H:%M:%S", t);
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUF_SIZE];
    char response[BUF_SIZE];
    socklen_t addr_len = sizeof(client_addr);

    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Server address setup
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("UDP Fruit Server listening on port %d...\n", PORT);

    while (1) {
        memset(buffer, 0, BUF_SIZE);
        recvfrom(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);

        printf("Request from %s:%d -> %s\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port),
               buffer);

        // Parse "fruit_name quantity"
        char fruit_name[20];
        int qty;
        if (sscanf(buffer, "%s %d", fruit_name, &qty) != 2) {
            snprintf(response, sizeof(response), "Invalid format. Use: fruit_name quantity");
            sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)&client_addr, addr_len);
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

                    // Add new customer if not seen
                    is_new_customer(client_addr);

                    snprintf(response, sizeof(response),
                             "Transaction successful!\n"
                             "You bought %d %s(s).\n"
                             "Remaining stock: %d\n"
                             "Last sold: %d at %s\n"
                             "Unique customers so far: %d\n",
                             qty, fruits[i].name, fruits[i].quantity,
                             fruits[i].last_sold, fruits[i].timestamp, customer_count);
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

        sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)&client_addr, addr_len);
    }

    close(sockfd);
    return 0;
}
