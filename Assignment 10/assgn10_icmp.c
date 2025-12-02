#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <sys/time.h>

unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1) sum += *(unsigned char*)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return ~sum;
}

int main() {
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    char packet[64];
    memset(packet, 0, sizeof(packet));

    struct icmphdr *icmp = (struct icmphdr*) packet;
    struct timeval tv;

    icmp->type = 13;   // Timestamp request
    icmp->code = 0;
    icmp->un.echo.id = htons(1);
    icmp->un.echo.sequence = htons(1);

    gettimeofday(&tv, NULL);
    unsigned long ms = (tv.tv_sec % 86400) * 1000 + (tv.tv_usec / 1000);

    memcpy(packet + sizeof(struct icmphdr), &ms, 4);

    icmp->checksum = checksum(packet, sizeof(packet));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("10.0.0.2");

    if (sendto(sock, packet, sizeof(packet), 0,
               (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("sendto");
    } else {
        printf("ICMP Timestamp request sent.\n");
    }

    close(sock);
    return 0;
}

