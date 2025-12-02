#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#include <netinet/ip_icmp.h>
#include <time.h>

int main() {
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle = pcap_open_offline("root_capture.pcap", errbuf);

    if (!handle) {
        printf("Error opening pcap file: %s\n", errbuf);
        return 1;
    }

    struct pcap_pkthdr *header;
    const u_char *data;
    int packet_count = 0;

    while (pcap_next_ex(handle, &header, &data) > 0) {
        packet_count++;

        printf("\n=== Packet %d ===\n", packet_count);

        // Timestamp
        printf("Time: %ld.%06ld sec\n", header->ts.tv_sec, header->ts.tv_usec);

        // L2 header (Ethernet)
        struct ether_header *eth = (struct ether_header *)data;
        printf("L2: Ethernet | Src MAC: %02x:%02x:%02x:%02x:%02x:%02x | ",
            eth->ether_shost[0], eth->ether_shost[1], eth->ether_shost[2],
            eth->ether_shost[3], eth->ether_shost[4], eth->ether_shost[5]);
        printf("Dst MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
            eth->ether_dhost[0], eth->ether_dhost[1], eth->ether_dhost[2],
            eth->ether_dhost[3], eth->ether_dhost[4], eth->ether_dhost[5]);

        if (ntohs(eth->ether_type) == ETHERTYPE_IP) {
            printf("L3: IPv4\n");
            struct ip *iph = (struct ip *)(data + sizeof(struct ether_header));

            printf("   Src IP: %s\n", inet_ntoa(iph->ip_src));
            printf("   Dst IP: %s\n", inet_ntoa(iph->ip_dst));
            printf("   Protocol: %d\n", iph->ip_p);

            if (iph->ip_p == IPPROTO_ICMP) {
                struct icmphdr *icmp = (struct icmphdr *)(data + sizeof(struct ether_header) + iph->ip_hl * 4);

                printf("L4: ICMP\n");
                printf("   ICMP Type: %d\n", icmp->type);
                printf("   ICMP Code: %d\n", icmp->code);
            }
        }
    }

    pcap_close(handle);
    return 0;
}

