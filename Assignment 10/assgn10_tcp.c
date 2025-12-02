#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>

struct pseudo_header {
    u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t placeholder;
    u_int8_t protocol;
    u_int16_t tcp_length;
};

unsigned short checksum(const char *buf, unsigned size) {
    unsigned long sum = 0;
    while (size > 1) {
        sum += *(const unsigned short*)buf;
        buf += 2;
        size -= 2;
    }
    if (size) sum += *(const unsigned char*)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

int main() {
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock < 0) {
        perror("Socket error");
        return 1;
    }

    char datagram[4096];
    memset(datagram, 0, 4096);

    struct iphdr *iph = (struct iphdr*) datagram;
    struct tcphdr *tcph = (struct tcphdr*) (datagram + sizeof(struct iphdr));
    char *data = datagram + sizeof(struct iphdr) + sizeof(struct tcphdr);

    // Payload (roll number)
    strcpy(data, "CSM24020");   

    struct sockaddr_in dest;
    dest.sin_family = AF_INET;
    dest.sin_port = htons(80);
    dest.sin_addr.s_addr = inet_addr("10.0.0.2");  // Target

    // Fill IP Header
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = htons(sizeof(struct iphdr) + sizeof(struct tcphdr) + strlen(data));
    iph->id = htons(54321);
    iph->ttl = 255;
    iph->protocol = IPPROTO_TCP;
    iph->saddr = inet_addr("10.0.0.1");          // SOURCE
    iph->daddr = dest.sin_addr.s_addr;

    // TCP Header
    tcph->source = htons(1234);
    tcph->dest = htons(80);
    tcph->seq = htonl(1);
    tcph->ack_seq = 0;
    tcph->doff = 5;
    tcph->syn = 1;
    tcph->window = htons(5840);

    // TCP checksum calculation
    struct pseudo_header psh;
    psh.source_address = iph->saddr;
    psh.dest_address = iph->daddr;
    psh.placeholder = 0;
    psh.protocol = IPPROTO_TCP;
    psh.tcp_length = htons(sizeof(struct tcphdr) + strlen(data));

    char pseudo_packet[4096];
    memcpy(pseudo_packet, &psh, sizeof(psh));
    memcpy(pseudo_packet + sizeof(psh), tcph, sizeof(struct tcphdr) + strlen(data));

    tcph->check = checksum(pseudo_packet, sizeof(psh) + sizeof(struct tcphdr) + strlen(data));

    // Enable IP_HDRINCL
    int one = 1;
    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt");
        return 1;
    }

    // Send packet
    if (sendto(sock, datagram, ntohs(iph->tot_len), 0,
               (struct sockaddr*)&dest, sizeof(dest)) < 0) {
        perror("sendto failed");
    } else {
        printf("TCP raw packet sent with payload: %s\n", data);
    }

    close(sock);
    return 0;
}

