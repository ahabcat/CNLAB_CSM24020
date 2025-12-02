#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <math.h>

#define PORT 4000
#define BUF_SIZE 1024

double evaluate_expression(char *expr) {
    double x, y;
    char op[10];

    // -------- Handle unary operators --------
    if (sscanf(expr, "%s %lf", op, &x) == 2) {
        if (strcmp(op, "sin") == 0) return sin(x);
        if (strcmp(op, "cos") == 0) return cos(x);
        if (strcmp(op, "tan") == 0) return tan(x);
        if (strcmp(op, "log") == 0) return log(x);
        if (strcmp(op, "sqrt") == 0) return sqrt(x);
        if (strcmp(op, "inv") == 0) return 1.0 / x;
    }

    // -------- Handle binary x + y --------
    if (sscanf(expr, "%lf %s %lf", &x, op, &y) == 3) {
        if (strcmp(op, "+") == 0) return x + y;
        if (strcmp(op, "-") == 0) return x - y;
        if (strcmp(op, "*") == 0) return x * y;
        if (strcmp(op, "/") == 0) return x / y;
    }

    return NAN; // parse error
}

int main() {
    int sockfd;
    char buffer[BUF_SIZE];
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len = sizeof(cli_addr);

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    printf("UDP Scientific Calculator Server Started (port %d)\n", PORT);

    while (1) {
        memset(buffer, 0, BUF_SIZE);

        recvfrom(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *)&cli_addr, &cli_len);
        printf("Client sent: %s\n", buffer);

        double result = evaluate_expression(buffer);

        char response[BUF_SIZE];
        if (isnan(result))
            snprintf(response, sizeof(response), "Error: Invalid Expression");
        else
            snprintf(response, sizeof(response), "Result = %.6lf", result);

        sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)&cli_addr, cli_len);
    }

    close(sockfd);
    return 0;
}

