#include <pcap.h>
#include <stdio.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#include <netinet/ip_icmp.h>

int main() {
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle = pcap_open_offline("root_capture.pcap", errbuf);

    struct pcap_pkthdr *header;
    const u_char *data;

    printf("TIME DIAGRAM (PING Operation)\n");
    printf("---------------------------------------------\n");
    printf("Time(sec)      Packet Type\n");
    printf("---------------------------------------------\n");

    while (pcap_next_ex(handle, &header, &data) > 0) {
        struct ether_header *eth = (struct ether_header *)data;

        if (ntohs(eth->ether_type) == ETHERTYPE_IP) {
            struct ip *iph = (struct ip *)(data + sizeof(struct ether_header));

            // ICMP
            if (iph->ip_p == IPPROTO_ICMP) {
                struct icmphdr *icmp =
                    (struct icmphdr *)(data + sizeof(struct ether_header) +
                                       iph->ip_hl * 4);

                if (icmp->type == 8)
                    printf("%ld.%06ld   ICMP Echo Request\n",
                        header->ts.tv_sec, header->ts.tv_usec);

                else if (icmp->type == 0)
                    printf("%ld.%06ld   ICMP Echo Reply\n",
                        header->ts.tv_sec, header->ts.tv_usec);
            }
        }
    }

    return 0;
}

