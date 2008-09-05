//
// packetProtection.cpp - provide source authentication for "packet"
// structures.
//
// Key information format:
//    <ManetAddr>|<key data>
//    <ManetAddr>|<key data>
//    <ManetAddr>|<key data>
//         :         :
//
// '#' introduces a comment.
//
// Copyright 2006 Sparta Inc.
//
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <ctype.h>

#include "hashtable.h"
#include "des.h"
#include "packetProtection.h"
#include "protect_simple.h"
#include "rsa_key.h"
#include "sequence_number.h"
#include "unprotect_simple.h"

static const char *rcsid __attribute__ ((unused)) = "$Id: packetProtection.cpp,v 1.14 2007/04/26 20:22:56 dkindred Exp $";

/* old cygwin releases don't have inet_pton */
static int my_inet_pton(int af, const char *src, void *dst)
{
	if (af == AF_INET)
	{
		return inet_aton(src, (struct in_addr *) dst) ? 1 : 0;
	}
	else
	{
#ifdef EAFNOSUPPORT
		errno = EAFNOSUPPORT;
#else
		errno = EINVAL;
#endif
		return -1;
	}
}

typedef struct TupPriv
{
	ManetAddr addr;
	nnp_sequence_number_t seq;
	rsa_private_key priv;
} TupPriv;

typedef struct TupPub
{
	ManetAddr addr;
	nnp_sequence_number_t seq;
	rsa_public_key pub;
	nnp_sequence_number_record seq_rec;
} TupPub;

static nnp_sequence_number_t TupPrivSeq(TupPriv *tp)
{
	return tp->seq=nnp_sequence_number_next(tp->seq);
}

struct PacketProtection
{
    char const *magic;
    int do_protection_flag;
    hashtable *priv;
    hashtable *pub;
};


static char const * const global_pp_magic = "PacketProtection";

//
// Parses the addr part of a line.
//
// Returns the broadcast addr, on invalid addr.
//
ManetAddr getAddr(char *addr, const char *fname, size_t lineno)
{
    ManetAddr ret;

        if(strchr(addr,':')==NULL)     /* if theres a :, its an IPv6 addr  */
        {
            struct in_addr tmp;
            if(my_inet_pton(AF_INET, addr, &tmp) > 0)
            {
                ret = ntohl(tmp.s_addr);
            }
            else
            {
		fprintf(stderr,"Invalid IPv4 address \"%s\". File %s, line %d\n",addr, fname, lineno);
		return NODE_BROADCAST;
            }
        }
        else
        {
#if 0
            struct in6_addr tmp;
            if(my_inet_pton(AF_INET, addr, &tmp) > 0)
            {
		fprintf(stderr,"IPv6 not supported, cannot convert \"%s\". File %s line %d\n", addr, fname, lineno );
		return NODE_BROADCAST;
            }
            else
#endif
            {
		fprintf(stderr,"Invalid IPv6 address \"%s\". File %s line %d\n", addr, fname, lineno );
		return NODE_BROADCAST;
            }
        }
    return ret;
} // getAddr

//
// Load a key from a line
//
// Throws a non-zero int on failure.
//
static int loadkey(
        struct PacketProtection *pp, 
        char *line, 
        char const *fname, 
        size_t lineno)
{
    char *key;
    ManetAddr addr;

    char *pos = strchr(line,'|');
    if (pos==NULL)
    {
	return -1;
    }
    *pos=0;
    key=pos+1;

    addr = getAddr(line, fname, lineno);

    if (addr==NODE_BROADCAST)
	return -1;

    rsa_public_key pub;
    rsa_private_key priv;
    char const *keytextbeg = key;
    char const *keytextend = strchr(key,0);

    int res = loadRSAPrivateKey(&priv, keytextbeg, keytextend);
    if(!res)
    {
	TupPriv *tp;
	tp=(TupPriv*)malloc(sizeof(*tp));
	tp->addr=addr;
	memcpy(&tp->priv,&priv,sizeof(priv));
	tp->seq=htonl(time(NULL) << 10);
	fprintf(stderr,"%s: initial sequence number= %u\n",__func__, ntohl(tp->seq));
	hashtableinsert(pp->priv,(char*)&addr,sizeof(addr),tp);
	fprintf(stderr,"inserting private key for %d.%d.%d.%d\n",PRINTADDR(addr));
    }
    else
    {
        res = loadRSAPublicKey(&pub, keytextbeg, keytextend);
        if(!res)
        {
		TupPub *tp;
		tp=(TupPub*)malloc(sizeof(*tp));
		tp->addr=addr;
		memcpy(&tp->pub,&pub,sizeof(pub));
		nnp_sequence_number_record_init(&tp->seq_rec);
		hashtableinsert(pp->pub,(char*)&addr,sizeof(addr),tp);
		fprintf(stderr,"inserting public key for %d.%d.%d.%d\n",PRINTADDR(addr));
        }
        else
        {
	    fprintf(stderr,"Invalid key data \"%s\" File %s line %d", line, fname, lineno);
	    return res;
        }
    }
    return 0;
} // loadkey

//
// Load keys from "in".
//
static int loadkeys(
        struct PacketProtection *pp,
        char const *fname)
{
	size_t lineno = 0;
	char line[1024];
	char *pos;
	FILE *fil;
	int ret=0;

	fil=fopen(fname,"r");
	if (fil==NULL)
		return -1;

        while(fgets(line,sizeof(line),fil))
        {
		lineno++;
		if (line[0]=='#')
			continue;

		pos=line;
		while((*pos!=0) && isspace(*pos))
			pos++;
		if (*pos==0)
			continue;
		if (loadkey(pp, pos, fname, lineno)!=0)     /* if key fails to parse...  */
			ret=-1;
	}
    return ret;
} // loadkeys

//
// Protect the packet.
//
// Returns zero on success, ENOENT on unknown source.
//
static int protect(
        struct PacketProtection const *pp, 
        packet **p_out, 
        ManetAddr from,
        packet const *p_in)
{
    int ret;
#if 0
    std::vector< PrivTup >::const_iterator it =
        find_if(pp->priv.begin(), pp->priv.end(),
                matchManetAddr(p_in->src));
#endif

    /* TOJ: why would this not be our private key?  Why do a lookup?
     */
    TupPriv *it=(TupPriv*)hashtablesearch(pp->priv, (char*)&(from), sizeof(from));

#ifdef DEBUG_PROTECTION
    fprintf(stderr,"protecting from %u.%u.%u.%u to %u.%u.%u.%u seq= %u\n",PRINTADDR(p_in->src),PRINTADDR(p_in->dst),ntohl(it->seq));
#endif

    if(it != NULL)
    {
        // make room for signature
        size_t payloaddelta =
            nnp_simple_additional_length_required(p_in->len, 0);
        packet *tmp = packetCopy(p_in, payloaddelta);
        if(tmp)
        {
            size_t len = p_in->len;
            if((ret = nnp_simple_protect(
                            &(it->priv), 
                            static_cast<uint8_t*>(tmp->data), 
                            static_cast<uint8_t*>(tmp->data), 
                            len,
                            0, TupPrivSeq(it), &len)) == 0)
            {
                if(len == static_cast<size_t>(tmp->len)) // paranoid
                {
                    // success
                    *p_out = tmp;
                }
                else
                {
			fprintf(stderr,"%s: Unexpected protected length %d, (expected %d) (src %d.%d.%d.%d dst %d.%d.%d.%d)\n", __func__, len, p_in->len, PRINTADDR(p_in->src), PRINTADDR(p_in->dst));
#if 0
                    std::cerr << __func__ << ": Unexpected protected length " 
                        << len << ", (expected " << p_in->len << ")"
                        << " (src " << PRINTADDRCPP(p_in->src) 
                        << " dst " << PRINTADDRCPP(p_in->dst) << ")"
                        << std::endl;
#endif
                    packetFree(tmp);
                    ret = -1;
                }
            }
            else
            {
		fprintf(stderr,"%s: Failed to protect packet (src %d.%d.%d.%d dst %d.%d.%d.%d)\n", __func__, PRINTADDR(p_in->src), PRINTADDR(p_in->dst));
#if 0
                std::cerr << __func__ 
                    << ": Failed to protect packet" 
                    << " (src " << PRINTADDRCPP(p_in->src) 
                    << " dst " << PRINTADDRCPP(p_in->dst) << ")"
                    << std::endl;
#endif
                packetFree(tmp);
            }
        }
        else
        {
		fprintf(stderr,"%s: Failed to copy packet (src %d.%d.%d.%d dst %d.%d.%d.%d\n",  __func__, PRINTADDR(p_in->src), PRINTADDR(p_in->dst));
#if 0
            std::cerr << __func__ << ": Failed to copy packet" 
                << " (src " << PRINTADDRCPP(p_in->src) 
                << " dst " << PRINTADDRCPP(p_in->dst) << ")"
                << std::endl;
#endif
            ret = -1;
        }
    }
    else
    {
	fprintf(stderr,"%s: No private key found for %d.%d.%d.%d\n",  __func__, PRINTADDR(p_in->src));
#if 0
        std::cerr << __func__ << ": No private key found for " 
            << PRINTADDRCPP(p_in->src) 
            << " (src " << PRINTADDRCPP(p_in->src) 
            << " dst " << PRINTADDRCPP(p_in->dst) << ")"
            << std::endl;
#endif
        ret = ENOENT;
    }
    return ret;
} // protect

//
// Unprotect the packet.
//
// Returns zero on succes, ENOENT on unknown source address, EILSEQ on
// modification.
//
int unprotect(struct PacketProtection const *pp, ManetAddr from, packet *p)
{
    int ret;
#if 0
    std::vector< PubTup >>::const_iterator it =
	find_if(pp->pub.begin(), pp->pub.end(),
		matchManetAddr(p->src));
#endif

    TupPub *it=(TupPub*)hashtablesearch(pp->pub, (char*)&(from), sizeof(from));

    if(it != NULL)
    {
        size_t len;
        NNPUnprotectResult res;
        switch(res = nnp_simple_unprotect(
                    &(it->pub), &(it->seq_rec),
                    static_cast<uint8_t*>(p->data),
                    static_cast<uint8_t*>(p->data),
                    p->len, &len, 0))
        {
            case NNPUnprotectValid:
                p->len = len;
                ret = 0;
                break;
            case NNPUnprotectModified:
                ret = EILSEQ;
                break;
            default:
#ifdef DEBUG_PROTECTION
		fprintf(stderr,"%s: %d.%d.%d.%d to %d.%d.%d.%d. %s\n", __func__, PRINTADDR(p->src), PRINTADDR(p->dst),NNP_UNPROTECT_RESULT_TEXT(res));
#endif
#if 0
		std::cerr << __func__ << ": " << PRINTADDRCPP(p->src)
			  << " to " << PRINTADDRCPP(p->dst) << ". "
			  << NNP_UNPROTECT_RESULT_TEXT(res) << std::endl;
#endif
                ret = -1;
        }
    }
    else
    {
	fprintf(stderr,"%s: No public key found for %d.%d.%d.%d (src %d.%d.%d.%d dst %d.%d.%d.%d)\n", __func__, PRINTADDR(p->src), PRINTADDR(p->src),PRINTADDR(p->dst));
#if 0
	std::cerr << __func__ << ": No public key found for "
		  << PRINTADDRCPP(p->src)
		  << " (src " << PRINTADDRCPP(p->src)
		  << " dst " << PRINTADDRCPP(p->dst) << ")"
		  << std::endl;
#endif
        ret = ENOENT;
    }
    return ret;
} // unprotect


////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

//
// Allocate and initialize packet protection If "fname" is null, then no
// protection will take place during "packetProtect()" and no
// verification will take place during "packetUnprotect()".
//
// pp    - struct PacketProtection that gets allocated and initialized.
//
// fname - file holding key information.
//
//     Key information format:
//        <ManetAddr>|<key data>
//        <ManetAddr>|<key data>
//        <ManetAddr>|<key data>
//             :         :
//
//     '#' introduces a comment.
//
// Returns zero on success.
//
int packetProtectionInit(
        struct PacketProtection **pp,
        char const *fname)
{
    int ret=0;
    *pp = (PacketProtection*)malloc(sizeof(**pp));
    memset(*pp, '\0', sizeof(**pp));
    (*pp)->magic = global_pp_magic;
    if(fname)
    {
            (*pp)->do_protection_flag = !0;

	    (*pp)->priv=hashtableinit(257,hashtablehash);
	    (*pp)->pub=hashtableinit(257,hashtablehash);

	    if ((ret=loadkeys(*pp, fname))!=0)
	    {
		hashtablefree((*pp)->priv);
		hashtablefree((*pp)->pub);
		free(*pp);
		return ret;
	    }
    }
    return ret;
} // packetProtectionInit 

//
// Clean up packet protection
//
void packetProtectionFini(struct PacketProtection *pp)
{
    if(pp && pp->magic == global_pp_magic)
    {
        pp->magic = 0;
        pp->do_protection_flag = 0;
	hashtablefree(pp->priv);
	hashtablefree(pp->pub);
	free(pp);
    }
    else if(pp)
    {
	fprintf(stderr,"%s received an invalid \"struct PacketProtection*\"\n",__func__);
    }
   return;
} // packetProtectionFini 

//
// Protect a packet.
//
// pp    - PacketProtection to use
//
// p_in  - Packet to protect
//
// p_out - Loaded with a new protected packet if packet protection
//         occurs, set to null if packet protection does not occur.
//
// Returns zero on success, ENOENT on unknown source.
//
int packetProtect(
        struct PacketProtection const *pp, 
        packet **p_out,
	ManetAddr from,
        packet const *p_in)
{
    int ret;
    if(pp && pp->magic == global_pp_magic)
    {
        if(pp->do_protection_flag)
        {
            ret = protect(pp, p_out, from, p_in);
        }
        else
        {
            *p_out = 0;
            ret = 0;
        }
    }
    else
    {
	fprintf(stderr,"%s: received a %s \"struct PacketProtection*\" (src %d.%d.%d.%d dst %d.%d.%d.%d)\n", __func__, (pp ? "n invalid" : " null"), PRINTADDR(p_in->src),PRINTADDR(p_in->dst));
        ret = -1;
    }
    return ret;
} // packetProtect

//
// Unprotect a protected packet
//
// Returns zero on succes, ENOENT on unknown source address, EILSEQ on
// modification.
//
int packetUnprotect(struct PacketProtection const *pp, ManetAddr from, packet *p)
{
    int ret;
    if(pp && pp->magic == global_pp_magic)
    {
        ret = pp->do_protection_flag ? unprotect(pp, from, p) : 0;
    }
    else
    {
	fprintf(stderr,"%s: received a %s \"struct PacketProtection*\" (src %d.%d.%d.%d dst %d.%d.%d.%d)\n", __func__, (pp ? "n invalid" : " null"), PRINTADDR(p->src),PRINTADDR(p->dst));
        ret = -1;
    }
    return ret;
} // packetUnprotect


#ifdef TEST_PACKET_PROTECTION

#include <sstream>

#define ADDR1 "192.168.1.1"
#define ADDR2 "192.168.2.1"
#define ADDR3 "192.168.1.2"
#define ADDR4 "192.168.1.3"	/* no public key in keystr */
#define ADDR5 "192.168.1.4"	/* no keys in keystr, public or private */

static char const *keystr =
"# private keys\n"
ADDR1 "|"
    "p9qe7ao5kCy75B2feymy5cv5v0liA7ga6jD8yDCC06l0obzvrwqz6g4clwfkp1eg5A657n7f1fD8pyewc8yiAlv32xyhfcfu|"
    "000003|"
    "jes1l0snq9eg|"
    "vhkvwwiD4lva2k3b9Bel43r2ebaEAhf8ko9Ci2zw8e70brmlyt2xkzplmr0cnux57g6r63A71jtosegbmf31rp2q1f0rrpDs|"
    "bqop4fq4w7vkrxol4E77wmC3m9Bcd8tdcozsbg4gznzlBm71|"
    "4AcEydwcbjrnAj25jsgxB9jkm7odmgqBvaqpswqup8zBha5C|"
    "xfw8jq49zEa29tuusx61ugsu7xc57cxwyym82wl0skd2v8aD|"
    "k51xymhgz57dimge3d4wlsD22jc4vyjmm2Bjwcsi4Ensbew0|"
    "fp7ur7llyEifAre73vb8CjEdr61q5n5o83r46yiuq7nCp6vc|"
    "83yzaalCCEwj\n"
ADDR2 "|"
    "Avzompfmz2xojtCn2s3vyd1zwb7ie2gkax1Ah9v2iwn0nBplj769f1emyvrabpqhtqhwsx2x029aEql6e7l4eqey6xwf6jon|"
    "000003|"
    "ibBAheorypjE|"
    "4lAtpi1Ehf8cA1j5n7g0jx5ki27l54rotgzfils5zvzm2ths3811mE2hjiCt84vbkv95CkAwimdyd9m2lk358pfrBi5g7Bhv|"
    "dBhBDc8aE930zCced1a0ozdBg6eprj6x0awym084wqz0gcAn|"
    "wtEo7rwiazytr3f6mC8B5eynj0ljzqthpdqcwe73s4ybd2e3|"
    "iaCCt0dsDuupffuzBpzByBejtaDttf1uchqqhl3vz2702qs5|"
    "9bbCDzhzpAxqnD89mej1fqEBavnh4q4m0789r14kcgndC8of|"
    "9j0qej9b86wli2a4fc5Ch9AtcrrEnvvE2clyln4twgmyA1n2|"
    "e8peqzz7ltC3\n"
ADDR3 "|"
    "A03uBnzyzj3bfbo252zdimBddCuisbiazgplvgxa7p2ra2ug04lEn1zqmprhqCe6obzidcE2uCvmlcrkp1pkzy59jfsj6xae|"
    "000003|"
    "owpa64tl13po|"
    "83lp8wegdbv1vkpiBxz1ohwye4m5tt9sg22clejAky84ymaqcfgh1cojmAvl4k2evfiazjaE56iBBtn4ADofus5zwd4ulq1t|"
    "0k9y9bwdoio7i7dknhAk9AA04yk9td0b98mq9t9gEru1uq4q|"
    "o2pjtose51Aqul5bB6b19n4pzqx5nbxE6gez4wja3qz59C0o|"
    "uiiBzmjkxAwhvzhvDCk9j51mwDab77lsB0ogqts619e6xe9x|"
    "0dk967lmtDtwAy76BAAs5s0DfnDBa707jx1hv7vvztk16v33|"
    "sgg2BsiAusoh7Ei4kj7ek1uhbgAroAz0h0rAfmpny2nh6pe2|"
    "0yrg17gwiBs1\n"
ADDR4 "|"
    "gcnmkkcdx2aB0ge2eosssbq57or0hetpt50Ekay6drcxwgBa3fB8gBc28bz96aipv49xDtCdmhy8rxs3ukx0sj1g3auv05ma|"
    "000003|"
    "2cp0cjh5ohjB|"
    "qq9zduw29m6bivC3fdpml83pkkz5w3a6i1c3rsd1nfsp63emk93jovyjzgw3qlckdykk3nBp40Ek0llabuf8sEnpk21bm8vr|"
    "0dzngiy2Cm90cbvwbqvv8sgonpdxfcs3C7rDehp5Eaxonu6l|"
    "6xwnh2olx3bBjegn7mdDC5iCavxdrlhauCja8mfpdtxCculj|"
    "gA5oywio9nuqlfB0hAo1zobfjo1f7snnEey2xehss0njwpc2"
    "|cnAj1oz2Ciackmkbc28xkfkhfumzBmv3om9bydsolvmgfxvr|"
    "gB75frgem27C0mbDeg9cD3qcjm7q8EA9557v5so395mpm6rE|"
    "8uivEk057vuA\n"
"# public keys\n"
ADDR1 "|"
    "p9qe7ao5kCy75B2feymy5cv5v0liA7ga6jD8yDCC06l0obzvrwqz6g4clwfkp1eg5A657n7f1fD8pyewc8yiAlv32xyhfcfu|"
    "000003|"
    "jes1l0snq9eg # public key data is just the first three fields of the private key data\n"
ADDR2 "|"
    "Avzompfmz2xojtCn2s3vyd1zwb7ie2gkax1Ah9v2iwn0nBplj769f1emyvrabpqhtqhwsx2x029aEql6e7l4eqey6xwf6jon|"
    "000003|"
    "ibBAheorypjE\n"
ADDR3 "|"
    "A03uBnzyzj3bfbo252zdimBddCuisbiazgplvgxa7p2ra2ug04lEn1zqmprhqCe6obzidcE2uCvmlcrkp1pkzy59jfsj6xae|"
    "000003|"
    "owpa64tl13po";

static ManetAddr strToManetAddr(char const *addrstr)
{
    struct in_addr addr;
    my_inet_pton(AF_INET, addrstr, &addr);
    return ntohl(addr.s_addr);
} // strToManetAddr

//
// Make a manetNode with enough pieces to do a packetMalloc().
//
static manetNode dummyManetNode(char const *addrstr)
{
    static struct manet dummymanet;
    dummymanet.curtime = 77;
    manetNode ret;
    ret.manet = &dummymanet;
    ret.addr = strToManetAddr(addrstr);
    return ret;
}

static manetNode dummyUs1 = dummyManetNode(ADDR1);
static manetNode dummyUs2 = dummyManetNode(ADDR2);
static manetNode dummyUs3 = dummyManetNode(ADDR3);
static manetNode dummyUs4 = dummyManetNode(ADDR4);
static manetNode dummyUs5 = dummyManetNode(ADDR5);

//
// Dummy calls to satisfy poor placement of functions in files.
//
#ifdef __cplusplus
extern "C" {
#endif
void nodeFree(manetNode*) { return; }
void dataInit(manetNode*) { return; }
void routeInit(manetNode*) { return; }
void floodInit(manetNode*) { return; }
void packetApiInit(manetNode*) { return; }
void nodeInit(manetNode*) { return; }
#ifdef __cplusplus
}
#endif

static void packetFill(packet *p)
{
    uint8_t *c = static_cast<uint8_t*>(p->data);
    uint8_t *e = c + p->len;
    while(c != e)
    {
        *c = 0x55;
        ++c;
    }
    return;
}

//
// Returns zero on success
//
static int testround(PacketProtection const *pp)
{
    int ret;
    fprintf(stderr,"test round trip: ");
    packet *p1o = packetMalloc(&dummyUs1, 300);
    packet *p2o = packetMalloc(&dummyUs1, 300);
    packet *p3o = packetMalloc(&dummyUs1, 300);
    if(p1o && p2o && p3o)
    {
        packet *p1;
        packet *p2;
        packet *p3;
        packetFill(p1o);
        packetFill(p2o);
        packetFill(p3o);
        if((ret = packetProtect(pp, &p1, dummyUs1.addr, p1o)) == 0 &&
           (ret = packetProtect(pp, &p2, dummyUs1.addr, p2o)) == 0 &&
           (ret = packetProtect(pp, &p3, dummyUs1.addr, p3o)) == 0)
        {
            if((ret = packetUnprotect(pp, dummyUs1.addr, p1)) == 0 &&
               (ret = packetUnprotect(pp, dummyUs1.addr, p2)) == 0 &&
               (ret = packetUnprotect(pp, dummyUs1.addr, p3)) == 0)
            {
                if(p1->len == p1o->len && 
                   memcmp(p1->data, p1o->data, p1->len) == 0 &&
                   p2->len == p2o->len && 
                   memcmp(p2->data, p2o->data, p2->len) == 0 &&
                   p3->len == p3o->len && 
                   memcmp(p3->data, p3o->data, p3->len) == 0)
                {
			fprintf(stderr,"Success\n");
                }
                else
                {
			fprintf(stderr,"Failure: Round trip doesn't match\n");
                    ret = -1;
                }
            }
            else
            {
		fprintf(stderr,"Failure\n");
            }
            packetFree(p1);
            packetFree(p2);
            packetFree(p3);
        }
        else
        {
		fprintf(stderr,"Failure\n");
        }
        packetFree(p1o);
        packetFree(p2o);
        packetFree(p3o);
    }
    else
    {
        ret = -1;
	fprintf(stderr,"Failure\n");
	fprintf(stderr,"%s: failed packetMalloc\n",__func__);
    }
    return ret;
}

//
// Returns zero on success
//
static int testmissingpub(PacketProtection const *pp)
{
    int ret;
    fprintf(stderr,"test missing public key: ");
    packet *po = packetMalloc(&dummyUs4, 300);
    if(po)
    {
        packet *p;
        packetFill(po);
        if((ret = packetProtect(pp, &p, dummyUs4.addr, po)) == 0)
        {
            if((ret = packetUnprotect(pp, dummyUs4.addr, p)) == ENOENT)
            {
		fprintf(stderr,"Success\n");
                ret = 0;
            }
            else
            {
		fprintf(stderr,"Failure\n");
                if(ret == 0)
                {
                    ret = -1;
                }
            }
            packetFree(p);
        }
        else
        {
	fprintf(stderr,"Failure\n");
        }
        packetFree(po);
    }
    else
    {
        ret = -1;
	fprintf(stderr,"Failure\n");
	fprintf(stderr,"%s: failed packetMalloc\n",__func__);
    }
    return ret;
}

//
// Returns zero on success
//
static int testmissingpriv(PacketProtection const *pp)
{
    int ret;
    fprintf(stderr,"test missing private key: ");
    packet *po = packetMalloc(&dummyUs5, 300);
    if(po)
    {
        packet *p;
        packetFill(po);
        if((ret = packetProtect(pp, &p, dummyUs5.addr, po)) == ENOENT)
        {
            fprintf(stderr,"Success\n");
            ret = 0;
	}
	else
	{
            fprintf(stderr,"Failure\n");
            fprintf(stderr,"%s: packetProtect returned %d\n",__func__, ret);
            if (ret == 0) packetFree(p);
            ret = -1;
        }
        packetFree(po);
    }
    else
    {
        ret = -1;
	fprintf(stderr,"Failure\n");
	fprintf(stderr,"%s: failed packetMalloc\n",__func__);
    }
    return ret;
}

//
// Returns zero on success
//
static int testspoof(PacketProtection const *pp)
{
    int ret;
    fprintf(stderr,"test spoof: ");
    packet *po = packetMalloc(&dummyUs2, 300);
    if(po)
    {
        packet *p;
        packetFill(po);
        if((ret = packetProtect(pp, &p, dummyUs2.addr, po)) == 0)
        {
            p->src = dummyUs3.addr; // '2' tries to spoof as '3'
            if((ret = packetUnprotect(pp, p->src, p)) == EILSEQ)
            {
		fprintf(stderr,"Success\n");
                ret = 0;
            }
            else
            {
		fprintf(stderr,"Failure (ret= %d)\n", ret);
                if(ret == 0)
                {
                    ret = -1;
                }
            }

        }
        else
        {
	    fprintf(stderr,"Failure\n");
        }
        packetFree(p);
    }
    else
    {
        ret = -1;
	fprintf(stderr,"Failure\n");
	fprintf(stderr,"%s: failed packetMalloc\n",__func__);
    }
    return ret;
}

//
// Returns zero on success
//
static int testmod(PacketProtection const *pp)
{
    int ret;
    fprintf(stderr,"test modified data: ");
    packet *po = packetMalloc(&dummyUs2, 300);
    if(po)
    {
        packet *p;
        packetFill(po);
        if((ret = packetProtect(pp, &p, dummyUs2.addr, po)) == 0)
        {
            ((uint8_t*)p->data)[66] ^= 0x40; // flip a bit
            if((ret = packetUnprotect(pp, dummyUs2.addr, p)) == EILSEQ)
            {
		fprintf(stderr,"Success\n");
                ret = 0;
            }
            else
            {
		fprintf(stderr,"Failure (ret= %d)\n", ret);
                if(ret == 0)
                {
                    ret = -1;
                }
            }
            packetFree(p);
        }
        else
        {
	    fprintf(stderr,"Failure\n");
        }
        packetFree(po);
    }
    else
    {
        ret = -1;
	fprintf(stderr,"Failure\n");
	fprintf(stderr,"%s: failed packetMalloc\n",__func__);
    }
    return ret;
} // testmod

//
// Returns zero on success
//
static int testnoprotection(void)
{
    int ret;
    struct PacketProtection *pp;
    fprintf(stderr,"test no protection: ");
    if((ret = packetProtectionInit(&pp, 0)) == 0)
    {
        packet *po = packetMalloc(&dummyUs2, 300);
        if(po)
        {
            packet *p;
            packetFill(po);
            if((ret = packetProtect(pp, &p, dummyUs2.addr, po)) == 0)
            {
                if(p == 0)
                {
                    if((ret = packetUnprotect(pp, dummyUs2.addr, po)) == 0)
                    {
			fprintf(stderr,"Success\n");
                    }
                    else
                    {
                        fprintf(stderr,"Failed, packetUnprotect\n");
                    }
                }
                else
                {
                    fprintf(stderr,"Failure, p_out not set to null\n");
                }
            }
            else
            {
		fprintf(stderr,"Failure, packetProtect() failed\n" );
            }
        }
        else
        {
            fprintf(stderr,"Failed packetMalloc\n");
        }
        packetProtectionFini(pp);
    }
    else
    {
	fprintf(stderr,"Failed packetProtectInit\n");
    }
    return ret;
} // testnoprotection

int main(int argc, char *argv[])
{
    int ret;
    char *line, *pos;
    int lineno=0;

    PacketProtection *pp = new PacketProtection;
    pp->magic = global_pp_magic;
    pp->do_protection_flag = !0;
    pp->priv=hashtableinit(257,hashtablehash);
    pp->pub=hashtableinit(257,hashtablehash);


	line=strdup(keystr);
	while(*line)
        {
		lineno++;
		pos=line;
		while((*line!=0) && (*line!='\n'))
			line++;
		if (*line!=0)
			line++;

		if ((pos[0]=='#'))
			continue;

		fprintf(stderr,"read line %d\n",lineno);

		while(isspace(*pos))
		pos++;
		if (loadkey(pp, pos, "keystr", lineno)!=0)     /* if key fails to parse...  */
			ret=-1;
	}


    if(testround(pp) == 0 &&
       testmissingpub(pp) == 0 &&
       testmissingpriv(pp) == 0 &&
       testspoof(pp) == 0 &&
       testmod(pp) == 0 &&
       testnoprotection() == 0)
    {
	fprintf(stderr,"Success\n");
        ret = 0;
    }
    else
    {
        fprintf(stderr,"FAILURE\n");
        ret = 1;
    }
    return ret;
} // main

#endif
