#ifndef PACKETSTATS_H_FILE
#define PACKETSTATS_H_FILE 1

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	PP_UNKNOWN = 0,
	PP_TCP,   /* port is MIN(src_port,dst_port) */
	PP_UDP,   /* port is MIN(src_port,dst_port) */
	PP_ICMP,
	PP_ARP,
	PP_IDS_HELLO,   /* PACKET_INTERIM2_HELLO packets; port=0 */
	PP_IDS_API,	/* PACKET_API (app messages); port=msg class */
	PP_IDS_ACK,	/* PACKET_DATA_ACK; port=0 */
	PP_IDS_OTHER,   /* other idsCommunications (UDP 4837) */
	PP_OTHER
} PacketProto;

struct PacketStats;

/* create an empty PacketStats struct */
struct PacketStats *createStats(void);

/* free PacketStats struct */
void destroyStats(struct PacketStats *);

/* record the given packet */
void recordPacket(struct PacketStats *stats,
		  PacketProto proto, unsigned int port,
		  unsigned int length);

/* write out a set of lines of the form
 * <PCLASS proto="(TCP|UDP|ICMP|ARP|IDS:*|API|ACK|OTHER)" port="123" packets="12" bytes="123"/>
 */
void dumpStats(const struct PacketStats *stats, FILE *f);

/* get the "protocol" name (e.g., "UDP", "IDS:API", etc.) */
const char *protoName(PacketProto proto);

unsigned long long getPacketCount(const struct PacketStats *stats, 
				  PacketProto proto, unsigned int port);
unsigned long long getByteCount(const struct PacketStats *stats, 
				PacketProto proto, unsigned int port);
unsigned long long getPacketCountTotal(const struct PacketStats *stats);
unsigned long long getByteCountTotal(const struct PacketStats *stats);

/* zero out all counters */
void clearStats(struct PacketStats *stats);

#ifdef __cplusplus
}
#endif

#endif /* PACKETSTATS_H_FILE */
