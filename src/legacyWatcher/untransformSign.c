/*
 * untransformSign.c - perform a sign untransformation 
 */
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

#include "mallocreadline.h"
#include "untransformSign.h"
#include "../protection/parse.h"
#include "../protection/rsa_key.h"
#include "../protection/sequence_number.h"
#include "../protection/unprotect_simple.h"

struct SignData
{
    const char const *magic;
    struct SignUntransformData *sud;
    uint8_t *raw;
    size_t rawLength;
    uint8_t *transformed;
    size_t transformedLength;
    ManetAddr signer;
    UntransformSignResult untransformResult;
};

typedef struct SignData *sptr;

#define TYPE sptr
#include "../protection/VEC.h"
#undef TYPE

typedef struct peer_entry
{
    struct in_addr addr;
    rsa_public_key key;
    nnp_sequence_number_record seq;
} peer_entry;

#define TYPE peer_entry
#include "../protection/VEC.h"
#undef TYPE

static const char const *globalMagic = "untransformSign";
static const int globalUntransformSignTag = 2;

struct SignUntransformData
{
    const char const *magic;
    sptr_vec store;
    peer_entry_vec peer;
};

/*********************************************************************/
/*********************************************************************/
/*********************************************************************/

#if 0
static void dump(char const *title, uint8_t const *buf, size_t len)
{
    size_t i = 1;
    uint8_t const *c = buf;
    uint8_t const *cend;
    char line[40];
    char *p = line;
    cend = c + len;
    printf("%s: dumping %d byte buffer\n", title, len);
    while(c != cend)
    {
        p += sprintf(p, "%02x", *c);
        if((i % 16) == 0)
        {
            printf("%s: %s\n", title, line);
            p = line;
        } else if((i % 4) == 0) {
            p += sprintf(p, " ");
        }
        ++i;
        ++c;
    }
    if((i - 1) % 16)
    {
        printf("%s: %s\n", title, line);
    }
    return;
} 
#endif

static int matchNptr(sptr const *ptr, void *check)
{
    return *ptr == check;
}

static int matchPeerAddr(peer_entry const *ptr, void *peer_addr)
{
    return memcmp(&(ptr->addr), peer_addr, sizeof(ptr->addr)) == 0;
}

/*
 * Create a handle to pass into the other functions.
 */
static UntransformDataHandle dataHandleCreate(struct Untransform *u)
{
    sptr ret;
    if(u && 
       u->untransformData &&
       ((struct SignUntransformData*)(u->untransformData))->magic == 
       globalMagic)
    {
        ret = (sptr)malloc(sizeof(*ret));
        if(ret)
        {
            ret->magic = globalMagic;
            ret->sud = (struct SignUntransformData*)u->untransformData;
            ret->raw = 0;
            ret->rawLength = 0;
            ret->transformed = 0;
            ret->transformedLength = 0;
            ret->signer = 0;
            ret->untransformResult = UntransformSignInternalError;
            push_back_sptr(&ret->sud->store, &ret);
        }
    }
    else
    {
        fprintf(stderr, "%s: null or invalid untransform\n", __func__);
        ret = 0;
    }
    return (UntransformDataHandle)ret;
}

/*
 * Set transformed data to untransform.
 *
 * Returns zero on success.
 */
static int setTransformed(
        UntransformDataHandle handle, 
        void const *transformed, 
        size_t transformedLength)
{
    int ret;
    sptr h = (sptr)handle;
    if(h && h->magic == globalMagic)
    {
        /*
         * Clean out old data. "transformed" and "raw" in the same 
         * buffer if "transformed" not null
         */
        if(h->transformed)
        {
            free(h->transformed);
            h->transformed = h->raw = 0;
            h->transformedLength = h->rawLength = 0;
        }
        else if(h->raw)
        {
            free(h->raw);
            h->raw = 0;
            h->rawLength = 0;
        }
        if(transformedLength > sizeof(struct in_addr))
        {
            struct in_addr peer_addr;
            peer_entry *peer;
            memcpy(&peer_addr, transformed, sizeof(peer_addr));
            h->signer = (ManetAddr)ntohl(peer_addr.s_addr);
            peer = find_peer_entry(&(h->sud->peer), matchPeerAddr, &peer_addr);
            if(peer)
            {
                size_t len = (sizeof(globalUntransformSignTag) + transformedLength)*2; 
                uint8_t *tmp = malloc(len);
                if(tmp)
                {
                    int ntag = htonl(globalUntransformSignTag);
                    size_t srclen = sizeof(ntag) + transformedLength;
                    uint8_t *dst = ((uint8_t*)tmp) + srclen;
                    size_t dstlen;
                    NNPUnprotectResult res;
                    nnp_sequence_number_t seq;
                    memcpy(tmp, &(ntag), sizeof(ntag)); // prepend with tag
                    memcpy(((uint8_t*)tmp) + sizeof(ntag), transformed, transformedLength);

                    if((res = nnp_simple_unprotect(
                                    &(peer->key),
                                    &(peer->seq),
                                    dst, (uint8_t*)tmp, srclen,
                                    &dstlen, 
                                    &seq)) == 0)
                    {
                        h->transformed = (uint8_t*)tmp;
                        h->transformedLength = srclen;
                        h->raw = dst;
                        h->rawLength = dstlen;
                        h->untransformResult = UntransformSignVerified;
                        ret = 0;
                    }
                    else
                    {
                        fprintf(stderr, "%s: Failed unprotect. \"%s\" (%d)\n",
                                __func__,
                                NNP_UNPROTECT_RESULT_TEXT(res), res);
                        h->untransformResult = res == NNPUnprotectReplayed ? 
                            UntransformSignReplay : UntransformSignVerificationFailure;
                        free(tmp);
                        ret = EINVAL;
                    }
                }
                else
                {
                    ret = ENOMEM;
                    fprintf(stderr, "%s: Failed to allocate %u bytes\n",
                            __func__, transformedLength);
                    h->untransformResult = UntransformSignInternalError;
                }
            }
            else
            {
                ret = EINVAL;
                fprintf(stderr, "%s: no key for peer %d.%d.%d.%d\n",
                        __func__,
                        peer_addr.s_addr & 0xFF, 
                        (peer_addr.s_addr & 0xFF00) >> 8, 
                        (peer_addr.s_addr & 0xFF0000) >> 16, 
                        (peer_addr.s_addr & 0xFF000000) >> 24);
                h->untransformResult = UntransformSignNoPeer;
            }
        }
        else
        {
            ret = EINVAL;
            fprintf(stderr, "%s: must have at least a %d byte peer address\n", 
                    __func__, sizeof(struct in_addr));
            h->untransformResult = UntransformSignInternalError;
            h->signer = 0;
        }
    }
    else
    {
        fprintf(stderr, "%s: null or invalid handle\n", __func__);
        ret = EINVAL;
    }
    return ret;
}

/*
 * Return the raw data from untransforming the data passed into
 * "setTransformed()"
 * 
 * Returns null on failure.
 */
static void const * raw(const UntransformDataHandle handle)
{
    void const *ret;
    sptr h = (sptr)handle;
    if(h && h->magic == globalMagic)
    {
        if(h->raw)
        {
            // don't include tag or peer address
            ret = h->raw + sizeof(globalUntransformSignTag) + sizeof(struct in_addr);
        }
        else
        {
            ret = 0;
            fprintf(stderr, "%s: attempt to retrieve unset data\n",
                    __func__);
        }
    }
    else
    {
        fprintf(stderr, "%s: null or invalid handle\n", __func__);
        ret = 0;
    }
    return ret;
} // raw

/*
 * Return the length of the buffer returned by "raw()" 
 *
 * Retuns zero on failure.
 */
static size_t rawLength(const UntransformDataHandle handle)
{
    size_t ret;
    sptr h = (sptr)handle;
    if(h && h->magic == globalMagic)
    {
        if(h->raw)
        {
            // subtract tag and peer address lengths
            ret = h->rawLength - sizeof(globalUntransformSignTag) - sizeof(struct in_addr);
        }
        else
        {
            fprintf(stderr, "%s: attempt to retrieve unset data\n",
                    __func__);
            ret = 0;
        }
    }
    else
    {
        fprintf(stderr, "%s: null or invalid handle\n", __func__);
        ret = 0;
    }
    return ret;
} // rawLength

/*
 * Return the transformed data passed into "setTransformed()"
 *
 * Retuns null on failure.
 */
static void const * transformed(const UntransformDataHandle handle)
{
    void const *ret;
    sptr h = (sptr)handle;
    if(h && h->magic == globalMagic)
    {
        if(h->transformed)
        {
            // don't include tag
            ret = h->transformed + sizeof(globalUntransformSignTag);
        }
        else
        {
            fprintf(stderr, "%s: attempt to retrieve unset data\n",
                    __func__);
            ret = 0;
        }
    }
    else
    {
        fprintf(stderr, "%s: null or invalid handle\n", __func__);
        ret = 0;
    }
    return ret;
} // transformed

/*
 * Return the length of the buffer returned by "transformed()" 
 *
 * Retuns zero on failure.
 */
static size_t transformedLength(const UntransformDataHandle handle)
{
    size_t ret;
    sptr h = (sptr)handle;
    if(h && h->magic == globalMagic)
    {
        if(h->transformed)
        {
            // subtract tag
            ret = h->transformedLength - sizeof(globalUntransformSignTag);
        }
        else
        {
            fprintf(stderr, "%s: attempt to retrieve unset data\n",
                    __func__);
            ret = 0;
        }
    }
    else
    {
        fprintf(stderr, "%s: null or invalid handle\n", __func__);
        ret = 0;
    }
    return ret;
} // transformedLength

/*
 * Cleans up resources owned by "handle". Note that the buffer
 * returned by "raw()" or "transformed()" is also cleaned up.
 */
static void dataHandleDestroy(UntransformDataHandle handle)
{
    sptr h = (sptr)handle;
    if(h && h->magic == globalMagic)
    {
        sptr *check = find_sptr(&h->sud->store, matchNptr, h);
        if(check)
        {
            remove_sptr(&h->sud->store, check);
        }
        else
        {
            fprintf(stderr, "%s: Somehow got what looks to be a valid "
                    "handle but I can't find it in the store.\n",
                    __func__);
        }
        h->magic = 0;
        if(h->transformed)
        {
            free(h->transformed);
        }
        else if(h->raw)
        {
            free(h->raw);
        }
        free(h);
    }
    else
    {
        fprintf(stderr, "%s: null or invalid handle\n", __func__);
    }
    return;
} // dataHandleDestroy

/*
 * Cleans up resources owned by "untransform". Only works
 * on an Untransform like "u->untransformDestroy(u)" - constructs
 * like "u1->untransformDestroy(u2)" will likely fail. Do not use
 * the Untransform after passing it into a untransformDestroy()
 * function.
 */
static void untransformDestroy(struct Untransform *untransform)
{
    if(untransform)
    {
        struct SignUntransformData *sud = 
            ((struct SignUntransformData*)untransform->untransformData);
        if(sud && sud->magic == globalMagic)
        {
            if(sud->store.end != sud->store.beg)
            {
                fprintf(stderr, "%s: cleaning %d outstanding handles "
                        "upon destroy\n",
                        __func__, sud->store.end - sud->store.beg);
            }
            while(sud->store.end != sud->store.beg)
            {
                dataHandleDestroy((UntransformDataHandle)*(sud->store.beg));
            }
            clean_sptr_vec(&sud->store);
            sud->magic = 0;
            clean_peer_entry_vec(&sud->peer);
            free(untransform);
        }
        else
        {
            fprintf(stderr, "%s: Attempt to destroy invalid untransform\n",
                    __func__);
        }
    }
    return;
} // transformDestroy

/*********************************************************************/
/*********************************************************************/
/*********************************************************************/

/*
 * Reads the file and uses every line found with the format:
 *
 * d.d.d.d <N>|<e>|<chk1><blah>
 *
 *   d.d.d.d - dotted decimal IPv4 address to use as an identifier.
 *   N       - modulus
 *   e       - public exponent (unused)
 *   chk1    - sha256-64 of N and e
 *   blah    - can be anything that is not chk1
 *
 * The formats of "<N>", "<e>", and "<chk1>" are as understood by
 * "string_to_fp_int()".
 *
 * The '#' character starts a comment which runs to the end of the line.
 */
Untransform *untransformSignCreate(char const *publicKeyFileName)
{
    Untransform *ret;
    FILE *fp = fopen(publicKeyFileName, "r");
    if(fp)
    {
        // append the untransform data to the end of the Untransform structure.
        ret = (Untransform*)malloc(sizeof(*ret) + sizeof(struct SignUntransformData));
        if(ret)
        {
            struct SignUntransformData *sud = 
                (struct SignUntransformData*)(ret + 1);
            ret->untransformData = sud;
            ret->tag = globalUntransformSignTag;
            ret->dataHandleCreate = dataHandleCreate;
            ret->setTransformed  = setTransformed;
            ret->raw = raw;
            ret->rawLength = rawLength;
            ret->transformed = transformed;
            ret->transformedLength = transformedLength;
            ret->dataHandleDestroy = dataHandleDestroy;
            ret->untransformDestroy = untransformDestroy;
            sud->magic = globalMagic;
            sud->store.beg = 0;
            sud->store.end = 0;
            sud->store.eos = 0;
            sud->peer.beg = 0;
            sud->peer.end = 0;
            sud->peer.eos = 0;
            for(;;)
            {
                char *line =  mallocreadnextnonblank(fp);
                peer_entry peer;
                char const *key;
                if(line)
                {
                    if((key = parseIPv4(&(peer.addr), line)) != 0 && 
                            (key = firstNonSpaceOf(key)) != 0)
                    {
                        int res;
                        char *end = strchr(key, 0);
                        if((res = loadRSAPublicKey(&(peer.key), key, end)) == 0)
                        {
                            nnp_sequence_number_record_init(&(peer.seq));
                            push_back_peer_entry(&(sud->peer), &peer);
                        }
                        else
                        {
                            fprintf(stderr, "%s: Error reading \"%s\" "
                                    "as public key, will continue "
                                    "parsing. \"%s\" (%d)\n", __func__, 
                                    line, strerror(res), res);
                        }
                    }
                    else
                    {
                        fprintf(stderr, "%s: Error reading \"%s\" as "
                                "IP address/public key, will continue "
                                "parsing.\n", __func__, line);
                    }
                    free(line);
                }
                else
                {
                    if(!feof(fp))
                    {
                        clean_peer_entry_vec(&(sud->peer));
                        free(ret);
                        ret = 0;
                        break;
                    }
                    break;
                }
            }
        }
        else
        {
            fprintf(stderr, "%s: Failed allocate %d bytes\n",
                    __func__,
                    sizeof(*ret) + sizeof(struct SignUntransformData));
        }
        fclose(fp);
    }
    else
    {
        ret = 0;
        fprintf(stderr, "%s: Failed to open \"%s\" for reading. "
                "\"%s\" (%d)\n", 
                __func__, publicKeyFileName, strerror(errno), errno);
    }
    return ret;
} // untransformSignCreate

/*
 * Return the signer.
 *
 * Returns 0 on failure. Failure occurs when the signer cannot be parsed
 * - this can happen on buffers that are too short.
 */
ManetAddr untransformSignSigner(UntransformDataHandle handle)
{
    ManetAddr ret;
    sptr h = (sptr)handle;
    if(h && h->magic == globalMagic)
    {
        ret = h->signer;
    }
    else
    {
        fprintf(stderr, "%s: null or invalid handle\n", __func__);
        ret = 0;
    }
    return ret;
} // untransformSignSigner

/*
 * Return the result of the untranform.
 *
 * Returns UntransformSignInternalError on invalid handle, buffer to be
 * verified was too short to be parsed, or failed memory allocation.
 */
UntransformSignResult 
untransformSignResult(UntransformDataHandle handle)
{
    UntransformSignResult ret;
    sptr h = (sptr)handle;
    if(h && h->magic == globalMagic)
    {
        ret = h->untransformResult;
    }
    else
    {
        fprintf(stderr, "%s: null or invalid handle\n", __func__);
        ret = UntransformSignInternalError;
    }
    return ret;
} // untransformSignResult
