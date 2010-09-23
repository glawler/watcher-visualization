/*
 * Based on code by:
 * Copyright (c) 2005 The Tcpdump Group
 * and Tim Carstens
 *
 * See: http://www.tcpdump.org/pcap.htm
 * and: http://www.tcpdump.org/sniffex.c
 *
 */
#include <string.h>
#include <stdlib.h>
#include <pcap.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <sys/types.h>

// watcher includes.
#include <libwatcher/client.h>
#include <libwatcher/edgeMessage.h>
#include <libwatcher/messageTypesAndVersions.h>    // for GUILayer
#include <libwatcher/sendMessageHandler.h>

watcher::Client *watcher_client=NULL;
struct in_addr localhost_addr;

/* default snap length (maximum bytes per packet to capture) */
#define SNAP_LEN 1518

/* ethernet headers are always exactly 14 bytes [1] */
#define SIZE_ETHERNET 14

/* Ethernet addresses are 6 bytes */
#define ETHER_ADDR_LEN	6

/* Typical size payload when using ping -p 'deadbeef' */
#define ICMP_PAYLOAD_LEN 56

/* Ethernet header */
struct sniff_ethernet {
        u_char  ether_dhost[ETHER_ADDR_LEN];    /* destination host address */
        u_char  ether_shost[ETHER_ADDR_LEN];    /* source host address */
        u_short ether_type;                     /* IP? ARP? RARP? etc */
};

/* IP header */
struct sniff_ip {
        u_char  ip_vhl;                 /* version << 4 | header length >> 2 */
        u_char  ip_tos;                 /* type of service */
        u_short ip_len;                 /* total length */
        u_short ip_id;                  /* identification */
        u_short ip_off;                 /* fragment offset field */
        #define IP_RF 0x8000            /* reserved fragment flag */
        #define IP_DF 0x4000            /* dont fragment flag */
        #define IP_MF 0x2000            /* more fragments flag */
        #define IP_OFFMASK 0x1fff       /* mask for fragmenting bits */
        u_char  ip_ttl;                 /* time to live */
        u_char  ip_p;                   /* protocol */
        u_short ip_sum;                 /* checksum */
        struct  in_addr ip_src,ip_dst;  /* source and dest address */
};
#define IP_HL(ip)               (((ip)->ip_vhl) & 0x0f)
#define IP_V(ip)                (((ip)->ip_vhl) >> 4)

/* ICMP header */
struct sniff_icmp {
    u_char type;
    u_char code;
    u_short checksum;
    u_char payload[];
};

/*
 * print data in rows of 16 bytes: offset   hex   ascii
 *
 * 00000   47 45 54 20 2f 20 48 54  54 50 2f 31 2e 31 0d 0a   GET / HTTP/1.1..
 */
void print_hex_ascii_line(const u_char *payload, int len, int offset)
{

    int i;
    int gap;
    const u_char *ch;

    /* offset */
    printf("%05d   ", offset);

    /* hex */
    ch = payload;
    for(i = 0; i < len; i++) {
        printf("%02x ", *ch);
        ch++;
        /* print extra space after 8th byte for visual aid */
        if (i == 7)
            printf(" ");
    }
    /* print space to handle line less than 8 bytes */
    if (len < 8)
        printf(" ");

    /* fill hex gap with spaces if not full line */
    if (len < 16) {
        gap = 16 - len;
        for (i = 0; i < gap; i++) {
            printf("   ");
        }
    }
    printf("   ");

    /* ascii (if printable) */
    ch = payload;
    for(i = 0; i < len; i++) {
        if (isprint(*ch))
            printf("%c", *ch);
        else
            printf(".");
        ch++;
    }

    printf("\n");

    return;
}

/*
 * print packet payload data (avoid printing binary data)
 */
void print_payload(const u_char *payload, int len)
{

    int len_rem = len;
    int line_width = 16;			/* number of bytes per line */
    int line_len;
    int offset = 0;					/* zero-based offset counter */
    const u_char *ch = payload;

    if (len <= 0)
        return;

    /* data fits on one line */
    if (len <= line_width) {
        print_hex_ascii_line(ch, len, offset);
        return;
    }

    /* data spans multiple lines */
    for ( ;; ) {
        /* compute current line length */
        line_len = line_width % len_rem;
        /* print line */
        print_hex_ascii_line(ch, line_len, offset);
        /* compute total remaining */
        len_rem = len_rem - line_len;
        /* shift pointer to remaining bytes to print */
        ch = ch + line_len;
        /* add offset */
        offset = offset + line_width;
        /* check if we have line width chars or less */
        if (len_rem <= line_width) {
            /* print last line and get out */
            print_hex_ascii_line(ch, len_rem, offset);
            break;
        }
    }

    return;
}

void showPingInWatcher(const struct in_addr &src, const struct in_addr &dst, char layer_data[5])
{
    watcher::event::EdgeMessagePtr em = watcher::event::EdgeMessagePtr(new watcher::event::EdgeMessage);
    em->node1=boost::asio::ip::address_v4(htonl(src.s_addr)); 
    em->node2=boost::asio::ip::address_v4(htonl(dst.s_addr)); 
    em->edgeColor=watcher::colors::red;
    em->expiration=5000;
    em->width=2;
    em->layer=std::string("ping_")+std::string(layer_data); 
    em->bidirectional=false;
    em->addEdge=true;

    bool was_connected=watcher_client->connected();
    while (!watcher_client->connected()) { 
        fprintf(stdout, "Attempting to connect to the watcher daemon...\n"); 
        watcher_client->connect(true); 
        if (!watcher_client->connected()) { 
            fprintf(stderr, "Unable to connect to the watcher daemon. Trying again in 2 seconds.\n"); 
            sleep(2); 
        }
    }
    if (!was_connected) { 
        fprintf(stdout, "Connected, but not sending out of date edge messages to the watcher.\n"); 
        return; 
    }
    else  {
        std::cout << "Sending edge message: " << *em;
        if(!watcher_client->sendMessage(em))
            fprintf(stderr, "Error sending edge message.\n"); 
    }
    return;
}

void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{
    static int count = 0;                   /* packet counter */

    /* declare pointers to packet headers */
    const struct sniff_ethernet *ethernet;  /* The ethernet header [1] */
    const struct sniff_ip *ip;              /* The IP header */
    const struct sniff_icmp *icmp;          /* The ICMP header */
    const u_char *payload;                    /* Packet payload */

    int size_ip;
    int size_payload;
    int size_layername;

    char src_addr_char_buf[INET_ADDRSTRLEN];
    char dst_addr_char_buf[INET_ADDRSTRLEN];

    printf("\nPacket number %d:\n", ++count);

    /* define ethernet header */
    ethernet = (struct sniff_ethernet*)(packet);

    /* define/compute ip header offset */
    ip = (struct sniff_ip*)(packet + SIZE_ETHERNET);
    size_ip = IP_HL(ip)*4;
    if (size_ip < 20) {
        printf("   * Invalid IP header length: %u bytes\n", size_ip);
        return;
    }

    if (ip->ip_p!=IPPROTO_ICMP) { 
        printf("Got non-ICMP packet for some reason. Should not happen.\n"); 
        return;
    }

    if (4 != (ip->ip_vhl >> 4)) {
        printf("We only understand ipv4 not ipv%d, sorry.\n", (ip->ip_vhl >> 4)); 
        exit (EXIT_FAILURE);
    }

    inet_ntop(AF_INET, &ip->ip_src, src_addr_char_buf, sizeof(src_addr_char_buf));
    inet_ntop(AF_INET, &ip->ip_dst, dst_addr_char_buf, sizeof(dst_addr_char_buf));

    printf("Found ping %s --> %s\n", src_addr_char_buf, dst_addr_char_buf); 

    icmp=(struct sniff_icmp*)(packet + SIZE_ETHERNET + size_ip);

    // The types we accept are somewhat arbitrary, just what seemed to work 
    // when I played around with ping -p
    struct in_addr from_addr, to_addr;
    switch (icmp->type) {
        case 0: printf("ping is echo reply\n"); 
                memcpy(&from_addr, &ip->ip_src, sizeof(from_addr)); 
                memcpy(&to_addr, &localhost_addr, sizeof(to_addr)); 
                break;
        case 8: printf("ping is echo request\n"); 
                memcpy(&to_addr, &localhost_addr, sizeof(from_addr)); 
                memcpy(&from_addr, &ip->ip_dst, sizeof(from_addr)); 
                break;
        default:
                printf("unhandled ping type\n"); 
                return;
                break;
    }

    payload=(const u_char*)&icmp->payload[0];

    // struct timeval is added by ping. Not sure what the extra 4 bytes are...
    payload+=sizeof(struct timeval)+4;
    size_payload=ICMP_PAYLOAD_LEN-(payload-icmp->payload); 
    // print_payload(payload, size_payload); 

    if (payload[0]==0xff && payload[1]==0xff) 
        printf("Found watcher-enabled ping packet\n"); 
    else 
        return;

    payload+=2;     // skip ffff

    // second four bytes are the layer name.
    const char hex_vals[]="0123456789abcdef"; 
    char layer_data[5]; 
    memset(layer_data, 0, sizeof(layer_data)); 
    for (int i=0; i<sizeof(layer_data)-1; i++) 
        layer_data[i]=(i%2==0) ? hex_vals[payload[i/2]>>4] : hex_vals[payload[i/2]&0x0f];
    
    showPingInWatcher(from_addr, to_addr, layer_data); 
}

int main(int argc, char **argv) 
{
    if (argc!=4) { 
        fprintf(stdout, "Usage: %s interface_name localhost_address watcherd_server_address\n", basename(argv[0])); 
        exit(EXIT_FAILURE);
    }

    char *dev=argv[1];

    if (inet_pton(AF_INET, argv[2], &localhost_addr)<=0) { 
        fprintf(stderr, "Error parsing %s as an ip address. It should be the address of the localhost on the interface given.\n"); 
        exit(EXIT_FAILURE);
    }

    watcher_client=new watcher::Client(argv[3]); 
    if (!watcher_client) {
        fprintf(stderr, "Error allocating new wathcer client connection.\n"); 
        exit (EXIT_FAILURE); 
    }
    while (!watcher_client->connected()) { 
        fprintf(stdout, "Attempting to connect to the watcher daemon...\n"); 
        watcher_client->connect(true); 
        if (!watcher_client->connected()) { 
            fprintf(stderr, "Unable to connect to the watcher daemon at %s. Trying again in 2 seconds.\n", argv[3]); 
            sleep(2); 
        }
    }
    fprintf(stdout, "Connected to the watcher daemon at %s.\n", argv[3]); 
    watcher_client->addMessageHandler(watcher::MultipleMessageHandler::create());

    char errbuf[PCAP_ERRBUF_SIZE]; 
    pcap_t *handle=NULL;
    
    char filter_exp[] = "icmp"; 
    
    struct bpf_program fp;
	bpf_u_int32 mask;
	bpf_u_int32 net;

    if (!dev) { 
        fprintf(stdout, "Did not find dev name on command line, attempting to find default device.\n"); 
		dev = pcap_lookupdev(errbuf);
		if (dev == NULL) {
			fprintf(stderr, "Couldn't find default device: %s\n", errbuf);
			exit(EXIT_FAILURE);
		}
    }

	if (pcap_lookupnet(dev, &net, &mask, errbuf) == -1) {
		fprintf(stderr, "Couldn't get netmask for device %s: %s\n", dev, errbuf);
		net = 0;
		mask = 0;
	}

    fprintf(stdout, "Sniffing on dev %s for ping packets.\n", dev); 

	/* open capture device */
	handle = pcap_open_live(dev, SNAP_LEN, 1, 1000, errbuf);
	if (handle == NULL) {
		fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuf);
		exit(EXIT_FAILURE);
	}

	/* make sure we're capturing on an Ethernet device [2] */
	if (pcap_datalink(handle) != DLT_EN10MB) {
		fprintf(stderr, "%s is not an Ethernet\n", dev);
		exit(EXIT_FAILURE);
	}

	if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1) {
		fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(handle));
		exit(EXIT_FAILURE);
	}

	if (pcap_setfilter(handle, &fp) == -1) {
		fprintf(stderr, "Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(handle));
		exit(EXIT_FAILURE);
	}

	pcap_loop(handle, -1, got_packet, NULL);

	pcap_freecode(&fp);
	pcap_close(handle);
}
