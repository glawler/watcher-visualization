// silliness required to get hash and hash_map from various gcc versions
#if __GNUC__ >= 3
# include <ext/hash_map>
# if __GNUC__ > 3 || __GNUC_MINOR__ >= 2
    using __gnu_cxx::hash_map;  // 3.2 and later
    using __gnu_cxx::hash;  // 3.2 and later
# else
    using std::hash_map;        // 3.0.1
    using std::hash;        // 3.0.1
# endif
#else
# include <hash_map>
#endif

#include "packetstats.h"

static const char *rcsid __attribute__ ((unused)) = "$Id: packetstats.cpp,v 1.4 2007/08/30 14:08:12 dkindred Exp $";

const char *
protoName(PacketProto proto)
{
	switch(proto)
	{
		case PP_UNKNOWN: return NULL;
		case PP_TCP:     return "TCP";
		case PP_UDP:     return "UDP";
		case PP_ICMP:    return "ICMP";
		case PP_ARP:     return "ARP";
		case PP_IDS_OTHER: return "IDS:OTHER";
		case PP_IDS_API:   return "IDS:API";
		case PP_IDS_ACK:   return "IDS:ACK";
		case PP_IDS_HELLO: return "IDS:HELLO";
		case PP_OTHER:   return "OTHER";
	}
	return NULL;
}

typedef struct PacketClass {
	PacketClass()
		: proto(PP_UNKNOWN), port(0)
	{ }

	PacketClass(PacketProto packetProto, unsigned int packetPort)
		: proto(packetProto), port(packetPort)
	{ }

	bool operator==(const PacketClass &other) const
	{
		return other.proto == proto && other.port == port;
	}
	PacketProto proto;
	unsigned int port;
} PacketClass;

struct PacketClassHash {
	size_t operator()(const PacketClass &pclass) const {
		hash<unsigned long> H;
		return H((pclass.proto << 24) | pclass.port);
	}
};


struct PacketCounts {
	unsigned long long packets;
	unsigned long long bytes;
};

typedef hash_map<PacketClass,PacketCounts,PacketClassHash> pclasses_t;

class PacketStats {
public:
	PacketStats()
		: pclasses(), totals()
	{
	}

	void clear()
	{
		pclasses.clear();
	}

	void dump(FILE *f) const
	{
		pclasses_t::const_iterator iter;
		/* XXX would be nice to sort these reasonably */
		for (iter = pclasses.begin(); iter != pclasses.end(); ++iter)
		{
			fprintf(f, " <PCLASS proto=\"%s\" port=\"%d\" packets=\"%llu\" bytes=\"%llu\"/>\n",
				protoName(iter->first.proto),
				(int) iter->first.port,
				iter->second.packets,
				iter->second.bytes);
		}
	}

	void getTotals(PacketCounts *outCounts)
		const
	{
		if (outCounts) *outCounts = totals;
	}

	void getCounts(PacketProto proto, unsigned int port, 
			PacketCounts *outCounts)
		const
	{
		if (outCounts)
		{
			pclasses_t::const_iterator iter
				= pclasses.find(PacketClass(proto,port));
			if (iter == pclasses.end())
			{
				outCounts->packets = 0;
				outCounts->bytes = 0;
			}
			else
			{
				*outCounts = iter->second;
			}
		}
	}

	void recordPacket(PacketProto proto, unsigned int port,
			  unsigned int length)
	{
		PacketCounts &counts
			= pclasses[PacketClass(proto,port)];
		++counts.packets;
		++totals.packets;
		counts.bytes += length;
		totals.bytes += length;
	}
private:
	pclasses_t pclasses;
	PacketCounts totals;
};


/*** C API ***/

struct PacketStats *createStats()
{
	return new PacketStats();
}

void destroyStats(struct PacketStats *stats)
{
	delete stats;
}

void recordPacket(struct PacketStats *stats,
		  PacketProto proto, unsigned int port,
		  unsigned int length)
{
	stats->recordPacket(proto, port, length);
}

void dumpStats(const struct PacketStats *stats, FILE *f)
{
	stats->dump(f);
}

void clearStats(struct PacketStats *stats)
{
	stats->clear();
}

unsigned long long getPacketCount(const struct PacketStats *stats,
                                  PacketProto proto, unsigned int port)
{
	PacketCounts counts;
	stats->getCounts(proto,port,&counts);
	return counts.packets;
}

unsigned long long getByteCount(const struct PacketStats *stats,
                                PacketProto proto, unsigned int port)
{
	PacketCounts counts;
	stats->getCounts(proto,port,&counts);
	return counts.bytes;
}

unsigned long long getPacketCountTotal(const struct PacketStats *stats)
{
	PacketCounts counts;
	stats->getTotals(&counts);
	return counts.packets;
}

unsigned long long getByteCountTotal(const struct PacketStats *stats)
{
	PacketCounts counts;
	stats->getTotals(&counts);
	return counts.bytes;
}
