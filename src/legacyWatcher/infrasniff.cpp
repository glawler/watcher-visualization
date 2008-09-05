#if __linux__
#define _BSD_SOURCE 1
#include <features.h>
#define __FAVOR_BSD 1 /* hack -- you're not supposed to set this. */
#endif
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <pcap.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netinet/ip_icmp.h>
#include <net/if.h>
#include <net/ethernet.h>     /* the L2 protocols */

#if __linux__
#define USE_PF_PACKET 1
#include <features.h>    /* for the glibc version number */
#ifndef __GLIBC_MINOR
#  ifdef __GLIBC_MINOR__
#    define __GLIBC_MINOR __GLIBC_MINOR__
#  else
#    warning don't have __GLIBC_MINOR defined
#  endif
#endif
#if __GLIBC__ >= 2 && __GLIBC_MINOR >= 1
#include <netpacket/packet.h>
#else
#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>   /* The L2 protocols */
#endif
#endif /* __linux__ */

#ifdef NNP
#include "protect.h"
#include "unprotect.h"
// #include "private_key.h"
#include "nnp_parse.h"
#include "processKeyFileCmd.h"
#include "sequence_number.h"
#endif
#include "packetstats.h"

// #include <libnet.h>

#ifndef MIN
#define MIN(a,b)  (((a)<(b))?(a):(b))
#endif

#include "des.h"
#include "flood.h"
#include "data.h"
#include "packetapi.h"
#include "apisupport.h"
#include "idsCommunications.h"
#include "interim2.h"
#include "marshal.h"
#ifdef NNP
#include "packetProtection.h"
#endif

static const char *rcsid __attribute__ ((unused)) = "$Id: infrasniff.cpp,v 1.15 2007/08/30 14:08:12 dkindred Exp $";

#define PRINTADDR(a) ((a)>>24)&0xFF,((a)>>16)&0xFF,((a)>>8)&0xFF,(a)&0xFF
#define PRINTETHERADDR(a) (a)[0] & 0xFF,(a)[1] & 0xFF,(a)[2] & 0xFF,(a)[3] & 0xFF,(a)[4] & 0xFF,(a)[5] & 0xFF

/* globals to simplify callbacks  */
pcap_t *phandle;
unsigned int showflags=0xFFFFFFFF;

static struct {
	struct PacketStats *packetstats;
	unsigned long window_size;
	time_t window_start;
	FILE *fd;
	/* for current packet */
	PacketProto current_proto; /* proto of the current packet */
	unsigned int current_port;  /* port of the current packet */
} stats;

char packetDescr[80*1024];
int showpacket;
int nnpKeyfile=0;
struct PacketProtection *packetProtection=NULL;
pcap_dumper_t *pcapfile=NULL;

#define SHOW_CLUSTERING		1
#define SHOW_OTHER		2
#define SHOW_API		4
#define SHOW_API_PAYLOAD	8

static void packetDescrAppend(char const *fmt, ...)
  __attribute__((format (printf, 1, 2)));
static void packetDescrAppend(char const *fmt, ...)
{
	char buff[80*1024];
	va_list arglist;
	va_start(arglist, fmt);
	vsprintf(buff, fmt, arglist);
	va_end(arglist);

	strcat(packetDescr,buff);
	return;
} /* defaultLogFunction */



void gotPacket(packet *p);

void gotAPI(packet *p)
{
	PacketApi *pa=packetApiUnmarshal(p,1);

	if (pa)
	{

		stats.current_proto = PP_IDS_API;
		stats.current_port = pa->type;

		packetDescrAppend("PACKET_API type 0x%x len= %d clen= %d dest= %d.%d.%d.%d type= %s ttl= %d\n",pa->type,pa->payloadLen, p->len -11,PRINTADDR(pa->origdest.addr), communicationsDestinationType2Str(pa->origdest.type),pa->origdest.ttl);
		if (!(showflags & SHOW_API))
			showpacket=0;
		if (showflags&SHOW_API_PAYLOAD)
			packetDescrAppend("payload: %.*s\n",pa->payloadLen,pa->payload);

		free(pa);
	}
	else
		packetDescrAppend("PACKET_API unmarshal failed\n");
}

void gotData(packet *p)
{
	PacketData *pd;
	int i;

	pd=packetDataUnmarshal(p);
	if (pd)
	{
		p->type=pd->origtype;
		p->len-=pd->len;

		packetDescrAppend("Data: type= 0x%x xmitnum= %d id= %u len= %d dst= ",pd->origtype,pd->xmitnum,pd->id,p->len);
		for(i=0;i<pd->destinationCount;i++)
			packetDescrAppend("%d.%d.%d.%d  ",PRINTADDR(pd->destinationList[i]));
		packetDescrAppend("\n");

		gotPacket(p);
		free(pd);
	}
	else
		packetDescrAppend("Data: unmarshal failed\n");
}

void gotFlood(packet *p)
{
#ifdef MODULE_FLOOD
	packetFlood *pf;
	pf=packetFloodUnmarshal(p);

        if (pf==NULL)
        {
		packetDescrAppend("Flood: unmarshal failed\n");
		return;
        }
	p->type=pf->origtype;
	p->src=pf->origsrc;
	p->dst=pf->origdst;
	p->len-=sizeof(*pf);

	packetDescrAppend("Flood: type= 0x%x src= %d.%d.%d.%d dst= %d.%d.%d.%d len= %d\n",p->type,PRINTADDR(p->src), PRINTADDR(p->dst),p->len);
	free(pf);
	gotPacket(p);
#else
	packetDescrAppend("Flood: [can't unmarshal because !MODULE_FLOOD]\n");
#endif
}

/* equivalent of realrecpacket in livenetwork.c
 */
void gotLive(const unsigned char *payload, int len, struct ip const * iph)
{
	const unsigned char *header=payload;
	packet *p = NULL;
	int olen;
	int version;
	int cryptflag;
	ManetAddr from;

	stats.current_proto = PP_IDS_OTHER;
	stats.current_port = 0; /* don't distinguish types */

	if (len < 8)
	{
		packetDescrAppend("%s: truncated packet (len= %d)\n", __func__, len);
		goto done;
	}

	p=(packet*)malloc(sizeof(*p));
	p->type= header[1];
	p->hopcount= header[2];
	p->len= len - 8;
	p->src=(header[3] << 24) | (header[4] <<16) | (header[5]<<8) | header[6];
	p->dst=ntohl(iph->ip_dst.s_addr);
	from=ntohl(iph->ip_src.s_addr);
	p->ttl= header[7];
	p->data=malloc(p->len);
        memcpy(p->data, payload+8, p->len);

	version=header[0] & 0x3F;
	cryptflag=header[0] & 0x40;
	olen=p->len;

	if (cryptflag)
	{
		if (packetProtection)
		{
			packetDescrAppend("packetProtection: encrypted\n");
#ifdef NNP
			if(packetUnprotect(packetProtection,from, p) != 0)
#endif
			{
				packetDescrAppend("%s: Failed unprotect of packet from %u.%u.%u.%u\n", __func__, PRINTADDR(p->src));
				goto done;
			}
		}
		else
		{
			packetDescrAppend("packetProtection: encrypted but no keys\n");
			goto done;
		}
	}

	packetDescrAppend("Livenetwork: origin= %d.%d.%d.%d len= %d payloadlen= %d version= %d type= %x\n",PRINTADDR(p->src),olen,p->len,header[0],p->type);

	gotPacket(p);
done:
        if (p)
	{
		free(p->data);
		free(p);
	}
}

void gotPacket(packet *p)
{
	switch(p->type)
	{
		case 0x22:      /* PACKET_API_RECEIVE  */
			gotAPI(p);
		break;
		case PACKET_DATA_DATA:
			gotData(p);
		break;
		case PACKET_DATA_ACK:
		{
			PacketDataAck *ackpd;

			ackpd=packetDataAckUnmarshal(p);
			/* we throw ACKs in with clustering packets, rather
			 * than make a new category */
			if (!(showflags&SHOW_CLUSTERING))
			{
				showpacket=0;
			}
			if (ackpd)
			{
				if (showpacket) packetDescrAppend("PACKET_DATA_ACK id= %u\n",ackpd->id);
				stats.current_proto = PP_IDS_ACK;
				stats.current_port = 0;
				free(ackpd);
			}
			else
			{
				if (showpacket) packetDescrAppend("PACKET_DATA_ACK unmarshal failed\n");
			}
		}
		break;
		case PACKET_FLOOD:    /* PACKET_FLOOD  */
			gotFlood(p);
		break;
		case 0xb5:    /* PACKET_INTERIM2_HELLO  */
			stats.current_proto = PP_IDS_HELLO;
			stats.current_port = 0;

			if (showflags&SHOW_CLUSTERING)
			{
				/* would be nice to put interim2's 
				 * helloUnmarshal/helloMarshal/etc. in an
				 * interim2hello.cpp that we could link with to
				 * avoid duplicating this stuff. */
				unsigned short optionsLen;
				unsigned char numNeighbors, level, totSymmetricNeighbors;
				unsigned long sequencenum;
				ManetAddr coordinator, desiredCoordinator;
				unsigned char *hp = (unsigned char*)p->data;
				if (p->len < 2+1+1+1+4+4+4)
				{
					packetDescrAppend("PACKET_INTERIM2_HELLO len= %d (truncated)\n",
							  p->len);
				}
				else
				{
					UNMARSHALSHORT(hp,optionsLen);
					UNMARSHALBYTE(hp,numNeighbors);
					UNMARSHALBYTE(hp,level);
					UNMARSHALBYTE(hp,totSymmetricNeighbors);
					UNMARSHALLONG(hp,sequencenum);
					UNMARSHALLONG(hp,coordinator);
					UNMARSHALLONG(hp,desiredCoordinator);
				/* ignore the rest */
					packetDescrAppend("PACKET_INTERIM2_HELLO len= %d numNeighbors= %d level= %d totSymmetricNeighbors= %d seqno= %lu coord= %d.%d.%d.%d desiredCoord= %d.%d.%d.%d\n",
							  p->len, numNeighbors, level,
							  totSymmetricNeighbors,
							  sequencenum,
							  PRINTADDR(coordinator),
							  PRINTADDR(desiredCoordinator));
				}
			}
			else
				showpacket=0;
		break;
		case 0xb6:    /* PACKET_INTERIM2_ROOT  */

			if (showflags&SHOW_CLUSTERING)
				packetDescrAppend("PACKET_INTERIM2_ROOT len= %d\n",p->len);
			else
				showpacket=0;
		break;
		default:
			packetDescrAppend("gotPacket: unknown packet type 0x%x\n",p->type);
		break;
	}
}

/* This function takes an /ethernet/ packet which contains an IP packet
 * It needs the ethernet packet because of the NNP stuff
 */
void ipPacketGot(const struct pcap_pkthdr *pheader, const unsigned char *etherpkt)
{
	const struct ip *iph = (const struct ip*)(etherpkt+14);
	const unsigned char *etherpktend = etherpkt + pheader->caplen;

	if (((const unsigned char *)(iph + 1)) > etherpktend)
	{
		packetDescrAppend("IP header truncated\n");
		return;
	}

#ifdef NNP
	if (iph->ip_p==253)
    {
        /* make a copy to un-nnp */
        uint8_t *etherpktcopy = (uint8_t*)malloc(pheader->caplen);
        if(etherpktcopy)
        {
            uint8_t const *end=(etherpktcopy+pheader->caplen);
            int nnp_offset;
            struct ip *iphcopy = (struct ip*)(etherpktcopy + 14);
            struct pcap_pkthdr pheadercopy;
            memcpy(etherpktcopy, etherpkt, pheader->caplen);
            memcpy(&pheadercopy, pheader, sizeof(pheadercopy));

            packetDescrAppend("NNP protected packet\n");
            if (!nnpKeyfile)
            {
                packetDescrAppend("no NNP keys\n");
                return;
            }

            NNPUnprotectResult res = nnp_unprotect(etherpktcopy, &end);
            if(res == NNPUnprotectValid)
            {
                /* Deprotected...  but still has NNP header.  */

                size_t iphlen = iphcopy->ip_hl << 2;
                const u_char *nnp = ((const u_char*)iphcopy) + iphlen;
                unsigned short origtotlen = ntohs(iphcopy->ip_len);
                u_char proto = nnp[0];
                unsigned short totlen = 
                    (((unsigned short)(*(nnp+2))) << 8) |
                    ((unsigned short)(*(nnp+3)));
                pheadercopy.caplen -= origtotlen - totlen;
                nnp_offset = 8;
                memmove(((unsigned char*)iphcopy) + nnp_offset - 14, ((unsigned
                                char*)iphcopy) - 14, iphlen + 14);
                /* fix up IP protocol, length */
                iphcopy = (struct ip*)(((unsigned char *)iphcopy) + nnp_offset);
                iphcopy->ip_len = htons(totlen);
                iphcopy->ip_p = proto;
                ipPacketGot(&pheadercopy, etherpktcopy + nnp_offset);
                free(etherpktcopy);
            }
            else
            {
                /* Failed deprotect */
                if (pcapfile!=NULL)
                {
                    pcap_dump((unsigned char*)pcapfile,  pheader, etherpkt);
                }
                packetDescrAppend(
                        "IP version: %d "
                        "ihl: %d "
                        "Id: %u "
                        "protocol: %d "
                        "src= %d.%d.%d.%d  "
                        "dst= %d.%d.%d.%d\n",
                        iph->ip_v, 
                        iph->ip_hl,
                        (int)iph->ip_id,
                        iph->ip_p,
                        PRINTADDR(ntohl(iph->ip_src.s_addr)),
                        PRINTADDR(ntohl(iph->ip_dst.s_addr)));
			    packetDescrAppend(
                        "Failed to remove NNP protection, error=\"%s\"\n", 
                        NNP_UNPROTECT_RESULT_TEXT(res));
			    if (!(showflags & SHOW_OTHER))
				    showpacket=0;
            }
        }
        return;
    }
#endif   /* NNP  */

	if (pcapfile!=NULL)
	{
		pcap_dump((unsigned char*)pcapfile,  pheader, etherpkt);
	}

	packetDescrAppend("IP version: %d ihl: %d Id: %u protocol: %d src= %d.%d.%d.%d  dst= %d.%d.%d.%d\n",iph->ip_v, iph->ip_hl,(int)iph->ip_id,iph->ip_p,PRINTADDR(ntohl(iph->ip_src.s_addr)),PRINTADDR(ntohl(iph->ip_dst.s_addr)));

	switch(iph->ip_p)
	{
		case IPPROTO_TCP:
		{
			int show = (showflags & SHOW_OTHER);
			const struct tcphdr *tcph=(const struct tcphdr *)((const char*)iph + (iph->ip_hl*4));

			if (!show) showpacket=0;

			stats.current_proto = PP_TCP;
			if (((const unsigned char *)(tcph + 1)) > etherpktend)
			{
				stats.current_port = 0;
				if (show) packetDescrAppend("TCP header truncated\n");
			}
			else
			{
				stats.current_port = 
					MIN(ntohs(tcph->th_sport), 
					    ntohs(tcph->th_dport));
				if (show) packetDescrAppend("TCP src port: %d dst port: %d\n",ntohs(tcph->th_sport),ntohs(tcph->th_dport));
			}
		}
		break;
		case IPPROTO_UDP:
		{
			int showother = (showflags & SHOW_OTHER);
			const struct udphdr *udph = (const struct udphdr *)((const char*)iph + (iph->ip_hl*4));

			stats.current_proto = PP_UDP;
			if (((const unsigned char *)(udph + 1)) > etherpktend)
			{
				stats.current_port = 0;
				if (showother) 
				{
					packetDescrAppend("UDP header truncated\n");
				}
				else
				{
					showpacket=0;
				}
			}
			else
			{
				stats.current_port = 
					MIN(ntohs(udph->uh_sport), 
					    ntohs(udph->uh_dport));

				packetDescrAppend("UDP src port: %d dst port: %d\n",ntohs(udph->uh_sport),ntohs(udph->uh_dport));

				if (udph->uh_dport==ntohs(4837))
				{
					const unsigned char *live=(const unsigned char *)udph+sizeof(*udph);
				
					gotLive(live,pheader->caplen - (live-etherpkt), iph);    /* UDP payload   */
				}
				else
				{
					if (!showother) showpacket=0;
				}
			}
		}
		break;
		case IPPROTO_ICMP:
		{
			const struct icmp *icmph;
			stats.current_proto = PP_ICMP;
			stats.current_port = 0;

			icmph=(const struct icmp *)((const char*)iph + (iph->ip_hl*4));
			if (((const unsigned char *)(icmph + 1)) > etherpktend)
			{
				packetDescrAppend("ICMP header truncated\n");
			}
			else
			{
				packetDescrAppend("ICMP type: %d code: %d\n",
						  icmph->icmp_type, icmph->icmp_code);
			}
			if (!(showflags&SHOW_OTHER))
				showpacket=0;
		}
		break;
		default:
			stats.current_proto = PP_OTHER;
			stats.current_port = 0;

			packetDescrAppend("Unknown protocol %d\n",iph->ip_p);
			if (!(showflags&SHOW_OTHER))
				showpacket=0;
		break;
	}
}

void checkStats(struct timeval now)
{
	if (stats.window_start == 0)
	{
		stats.window_start = now.tv_sec;
		/* round down to window boundary */
		stats.window_start -= stats.window_start % stats.window_size;
	}
	
	if (now.tv_sec >= (time_t)(stats.window_start + stats.window_size))
	{
		/* dump stats and start a new window */
		if (stats.fd)
		{
			fprintf(stats.fd,
				"<WINDOW starttime=\"%ld\" duration=\"%lu\" packets=\"%llu\" bytes=\"%llu\">\n",
				stats.window_start, 
				stats.window_size,
				getPacketCountTotal(stats.packetstats),
				getByteCountTotal(stats.packetstats)
				);
			dumpStats(stats.packetstats, stats.fd);
			fprintf(stats.fd, "</WINDOW>\n");
			fflush(stats.fd);
		}
		clearStats(stats.packetstats);
		stats.window_start += stats.window_size;
	}
}

/* callback function called by pcap every time we get a packet
*/
void pcapgotpacket(u_char *user,const struct pcap_pkthdr *pheader,const u_char *packet)
{
	packetDescr[0]=0;
	showpacket=1;
	const struct ether_header *ethh=(const struct ether_header*)packet;
	static int trunc_warning_given = 0;

	if (!trunc_warning_given && pheader->caplen != pheader->len)
	{
		fprintf(stderr, "WARNING: packets are truncated; may report incomplete information\n");
		trunc_warning_given = 1;
	}

	if (stats.packetstats)
	{
		checkStats(pheader->ts);
	}
	stats.current_proto = PP_OTHER;
	stats.current_port = 0;

	/* Is this packet actually IP?  
	*/

	switch(ntohs(ethh->ether_type))
	{
		case ETHERTYPE_IP:
			ipPacketGot( pheader, packet);
		break;
		case ETHERTYPE_ARP:
			stats.current_proto = PP_ARP;
			stats.current_port = 0;

			if (!(showflags&SHOW_OTHER))
				showpacket=0;
			if (pheader->caplen < 32)
			{
				packetDescrAppend("ARP truncated\n");
				break;
			}
			if ((((packet[14] << 8 ) | packet[15] ) !=1) && (((packet[16] << 8) | packet[17])!=0x800))
				break;
			switch((packet[20] << 8) | packet[21])
			{
				case 1: /* request */
					packetDescrAppend("ARP Request ");
				break;
				case 2: /* reply */
					packetDescrAppend("ARP Reply ");
				break;
			}
			packetDescrAppend("ARP sender IP= %d.%d.%d.%d MAC= %x:%x:%x:%x:%x:%x\n",packet[28],packet[29],packet[30],packet[31],packet[22],packet[23],packet[24],packet[25],packet[26],packet[27]);
		break;
		default:
			packetDescrAppend("Unknown Ether type\n");
			if (!(showflags&SHOW_OTHER))
				showpacket=0;
		break;
	}
	if (stats.packetstats) 
	{
		recordPacket(stats.packetstats, stats.current_proto, stats.current_port, pheader->len);
	}
}

void pcapgotpacketwrapper(u_char *user,const struct pcap_pkthdr *pheader, const u_char *packet)
{
	pcapgotpacket(user, pheader, packet);

	if (showpacket)
	{
		/* should check caplen */
		const struct ether_header *ethh=(const struct ether_header*)packet;
		fprintf(stdout,"\ntime= %d.%03d proto= %s port= %d len= %d caplen= %d ether proto= %x  src= %x:%x:%x:%x:%x:%x  dst= %x:%x:%x:%x:%x:%x\n",
			(int)(pheader->ts.tv_sec),
			(int)(pheader->ts.tv_usec/1000),
			protoName(stats.current_proto),
			stats.current_port,
			pheader->len, 
			pheader->caplen,
			ntohs(ethh->ether_type),
			PRINTETHERADDR(ethh->ether_shost),
			PRINTETHERADDR(ethh->ether_dhost));
		fprintf(stdout,"%s",packetDescr);

	}
}


int main(int argc, char *argv[])
{
	char err[1024];
	char *device="eth0";
	char *fname=NULL;
	int ch;
	int exitflag=0;
	int inflag=1;
	int outflag=1;
	FILE *infd=stdout;
	FILE *outfd=stdout;
	time_t timeout=-1;
#ifdef NNP
	char *keyfile=NULL;
	char *protectionKeyfile=NULL;
#endif
	char *pcapfilename=NULL;

	stats.window_size = 10; /* default 10s reporting interval */

	while ((ch = getopt(argc, argv, "r:i:d:vs:I:O:S:W:t:k:p:l:?")) != -1)
		switch (ch)
		{
			case 't':
				{
				int i;
				sscanf(optarg,"%d",&i);
				timeout=i;
				}
			break;
			case 'd':
				inflag=0;
				outflag=0;
				if (strstr(optarg,"i")!=NULL)
					inflag=1;
				if (strstr(optarg,"o")!=NULL)
					outflag=1;
			break;
			case 's':
				showflags=0;
				while(*optarg)
				{
					switch(*optarg)
					{
						case 'a':
							showflags|=SHOW_API;
						break;
						case 'p':
							showflags|=SHOW_API;
							showflags|=SHOW_API_PAYLOAD;
						break;
						case 'c':
							showflags|=SHOW_CLUSTERING;
						break;
						case 'o':
							showflags|=SHOW_OTHER;
						break;
						case '?':
						default:
							fprintf(stderr,"-s [pco]*\n"
								"a - show API (message) packets\n"
								"p - show payload of message packets\n"
								"c - show clustering and ack packets\n"
								"o - show other (non hierarchy) packets\n");
							exit(1);
						break;
					}
					optarg++;
				}
			break;
			case 'I':
				infd=fopen(optarg,"w");
				if (infd==NULL)
				{
					perror("can't open output file:");
					exit(1);
				}
			break;
			case 'O':
				outfd=fopen(optarg,"w");
				if (outfd==NULL)
				{
					perror("can't open output file:");
					exit(1);
				}
			break;
			case 'S':
				if (strcmp(optarg,"-") == 0)
				{
					stats.fd=stdout;
				}
				else
				{
					stats.fd=fopen(optarg,"w");
				}
				if (stats.fd==NULL)
				{
					perror("can't open stats file:");
					exit(1);
				}
				stats.packetstats = createStats();
			break;
			case 'W':
			{
				char *endp;
				stats.window_size = strtoul(optarg, &endp, 10);
				if (*optarg == '\0' || *endp != '\0')
				{
					fprintf(stderr, "argument to -W (%s) must be positive number of seconds\n", optarg);
					exit(1);
				}
			}
			break;
			case 'r':
				fname=strdup(optarg);
			break;
			case 'p':
				pcapfilename=strdup(optarg);
			break;
			case 'i':
				device=strdup(optarg);
			break;
			case 'k':
#ifdef NNP
				keyfile=strdup(optarg);
#endif
			break;
			case 'l':
#ifdef NNP
				protectionKeyfile=strdup(optarg);
#endif
			break;
			case '?':
			default:
				fprintf(stderr,"%s: a packet sniffer for idsCommunications\n"
					"-r filename - read traffic from a pcap file (from tcpdump)\n"
					"-i interface - sniff traffic from named interface\n"
					"-s [pco] - show selected packets (Default all, specify ? to get list)\n"
					"-I filename - specify a file to write inbound packets to\n"
					"-O filename - specify a file to write outbound packets to\n"
					"-t int - capture packets for int seconds\n"
#ifdef NNP
					"-k filename - file of keys for NNP support\n"
					"-l filename - file of keys for packetProtection support\n"
#endif
					"-d [i][o] - direction of packets i=incoming o=outgoing (Linux live only)\n"
					"-S filename - log periodic stats to the given file (\"-\" for stdout)\n"
					"-W seconds - use the given window for periodic stats\n"
					"-p filename - output all packets in pcap format to file\n"
					, argv[0]);
				exit(1);
			break;
		}

#ifdef NNP
		/* NNP init:   */
		if (keyfile)
		{
			char line[8192];
			FILE *fil;

			fil=fopen(keyfile,"r");
			if (fil!=NULL)
			{
				while(fgets(line,sizeof(line),fil))
				{
					line[strlen(line)-1]=0;
					processKeyFileCmd(line);
				}
				fclose(fil);
				nnpKeyfile=1;
			}
			/* we want to decode even duplicate packets  */
			nnp_set_replay_check_enabled(0);
		}
		if (protectionKeyfile)
		{
			packetProtectionInit(&packetProtection, protectionKeyfile);
			nnp_set_replay_check_enabled(0);
		}
		else
			packetProtection=NULL;

#endif

	/* We can only select on direction if using PF_PACKET interface */
	if ((inflag == 0 || outflag == 0)
#if USE_PF_PACKET
	    && fname != NULL
#endif
	    )
	{
		fprintf(stderr, "error: can't use -d with pcap live or offline\n");
		exit(1);
	}

	if (pcapfilename!=NULL)
	{
		pcap_t *fil;

		fil=pcap_open_dead(DLT_EN10MB, 2048);
		pcapfile=pcap_dump_open(fil,pcapfilename);
	}


/* pcap implementation:
 */

	if (fname==NULL)
	{
#if !USE_PF_PACKET
		/* do live  */
		phandle=pcap_open_live(device,2048,1,250,err);

		if (phandle==NULL)
		{
			fprintf(stderr,"%s: %s\n",argv[0],err);
			exit(1);
		}
		while(!exitflag)
		{
			pcap_dispatch(phandle,-1,pcapgotpacketwrapper,NULL);
		}
#else /* USE_PF_PACKET */
		time_t starttime;
		int s, size;
		struct sockaddr_ll from;
		socklen_t fromlen;
		int interface=-1;
		char buffer[65536];
		struct pcap_pkthdr pheader;

		struct ifconf       ifc; 
		struct ifreq        ifreq, *ifr;
		char buff[1024];

		/* PF_PACKET implementation, to be able to discern incoming from outgoing
		 */

		s = socket(PF_PACKET, SOCK_RAW,htons(ETH_P_ALL));
		if( s == -1)
		{
			perror("socket");
			exit(1);
		}

		ifc.ifc_len=sizeof(buff);
		ifc.ifc_buf=buff;

		if (ioctl(s, SIOCGIFCONF, (char *)&ifc)>=0)
		{
#ifdef linux
			for(ifr=ifc.ifc_req;ifr<(struct ifreq *)((char*)ifc.ifc_req+ifc.ifc_len);ifr=(struct ifreq *)(((char*)ifr)+(sizeof(*ifr))))
#else
			for(ifr=ifc.ifc_req;ifr<(struct ifreq *)((char*)ifc.ifc_req+ifc.ifc_len);ifr=(struct ifreq *)(((char*)ifr)+(ifr->ifr_addr.sa_len + IFNAMSIZ)))
#endif
			{
				ifreq = *ifr;


				if (strcmp(device,ifr->ifr_ifrn.ifrn_name)==0)
				{
					if (ioctl(s, SIOCGIFINDEX, (char *)&ifreq) < 0) 
						continue;
					fprintf(stderr,"interface %s  index %d\n",ifr->ifr_ifrn.ifrn_name,ifreq.ifr_ifindex);
					interface=ifreq.ifr_ifindex;
				}
			}
		}

		if (interface<0)
		{
			fprintf(stderr,"interface %s not found\n",device);
			exit(1);
		}

		{
			struct packet_mreq req;
			memset(&req,0,sizeof(req));
			req.mr_ifindex=interface;
			req.mr_type=PACKET_MR_PROMISC;
			req.mr_alen=0;

			errno=0;
			if (setsockopt(s,SOL_PACKET,PACKET_ADD_MEMBERSHIP,&req,sizeof(req))<0)
			{
				perror("setsockopt:");
				exit(1);
			}
		}

		gettimeofday(&(pheader.ts),NULL);
		starttime=pheader.ts.tv_sec;

		while(!exitflag)
		{
			FILE *outputfd=NULL;
			fromlen = sizeof(from);
			size=recvfrom(s,buffer,65536, 0,(struct sockaddr*) &from,&fromlen);

			pheader.len=size;
			pheader.caplen=size;
			gettimeofday(&(pheader.ts),NULL);
			if ((timeout>0) && pheader.ts.tv_sec>(starttime+timeout))
				exitflag=1;

#if 1
			// printf("interface %d type %d\n",from.sll_ifindex,from.sll_pkttype);
			if (from.sll_ifindex!=interface)
				continue;
#endif
			switch(from.sll_pkttype)	/* see /usr/include/linux/if_packet.h */
			{
				case PACKET_OUTGOING:
					if (outflag) 
						pcapgotpacket(NULL,&pheader,(u_char*)buffer);
					outputfd=outfd;
				break;
				case PACKET_BROADCAST:
				default:
					if (inflag)
						pcapgotpacket(NULL,&pheader,(u_char*)buffer);
					outputfd=infd;
				break;
			}
			if (showpacket)
				fprintf(outputfd,"%s",packetDescr);
		}
#endif /* USE_PF_PACKET */

	}
	else
	{
		phandle=pcap_open_offline(fname,err);

		if (phandle==NULL)
		{
			fprintf(stderr,"%s: %s\n",argv[0],err);
			exit(1);
		}

		/* fprintf(stderr,"open successful...\n"); */
		fprintf(stderr,"reading from %s\n",fname);
		while(pcap_dispatch(phandle,-1,pcapgotpacketwrapper,NULL)!=0)
		{
		}
		pcap_close(phandle);
		if (stats.packetstats)
		{
			/* dump one last set of packet stats */
			struct timeval t = {LONG_MAX, 999999};
			checkStats(t);
		}
	}

	if (pcapfile)
		pcap_dump_close(pcapfile);
	return(0);
}


void nodeInit(manetNode *us)
{
}

void nodeFree(manetNode *us)
{
}

void packetSend(manetNode *us, packet *p, int origflag)
{
	fprintf(stderr,"This is infrasniff, this shouldn't be called\n");
	abort();
}

