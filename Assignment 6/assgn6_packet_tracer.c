#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <net/ethernet.h>
#include <sys/socket.h>
#include <netpacket/packet.h>

int main() {
    int sockfd;
    unsigned char buffer[65536];

    // Create raw socket
    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd < 0) {
        perror("Socket error");
        return 1;
    }

    printf("TCP Packet Analyzer Started...\n");

    while (1) {
        int data_size = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);
        if (data_size < 0) {
            perror("Recv error");
            return 1;
        }

        // -------- L2 Ethernet Header --------
        struct ethhdr *eth = (struct ethhdr *)buffer;

        if (ntohs(eth->h_proto) != ETH_P_IP)
            continue; // ignore non-IP packets

        // -------- L3 IP Header --------
        struct iphdr *ip = (struct iphdr*)(buffer + sizeof(struct ethhdr));

        // We only want TCP packets
        if (ip->protocol != IPPROTO_TCP)
            continue;

        unsigned short iphdr_len = ip->ihl * 4;

        // -------- L4 TCP Header --------
        struct tcphdr *tcp = (struct tcphdr*)(buffer + sizeof(struct ethhdr) + iphdr_len);

        // Print Packet Info
        printf("\n=== TCP Packet ===\n");

        printf("Ethernet: SRC MAC %02X:%02X:%02X:%02X:%02X:%02X ",
            eth->h_source[0], eth->h_source[1], eth->h_source[2],
            eth->h_source[3], eth->h_source[4], eth->h_source[5]);

        printf("DEST MAC %02X:%02X:%02X:%02X:%02X:%02X\n",
            eth->h_dest[0], eth->h_dest[1], eth->h_dest[2],
            eth->h_dest[3], eth->h_dest[4], eth->h_dest[5]);

        struct in_addr s, d;
        s.s_addr = ip->saddr;
        d.s_addr = ip->daddr;

        printf("IP: SRC %s  DEST %s\n", inet_ntoa(s), inet_ntoa(d));
        printf("TCP: SRC PORT %u  DEST PORT %u\n", ntohs(tcp->source), ntohs(tcp->dest));
        printf("SEQ: %u  ACK: %u\n", ntohl(tcp->seq), ntohl(tcp->ack_seq));
        printf("FLAGS: SYN=%d ACK=%d FIN=%d RST=%d PSH=%d\n",
            tcp->syn, tcp->ack, tcp->fin, tcp->rst, tcp->psh);

        printf("===================\n");
    }

    close(sockfd);
    return 0;
}
