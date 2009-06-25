/*
 * transformSign.c - perform a signature transformation 
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "mallocreadline.h"
#include "transformSign.h"
#include "../protection/parse.h"
#include "../protection/protect_simple.h"
#include "../protection/rsa_key.h"
#include "../protection/sequence_number.h"
#include "../protection/unprotect_simple.h"

struct SignData
{
    const char const *magic;
    struct SignTransformData *std;
    uint8_t *raw;
    size_t rawLength;
    uint8_t *transformed;
    size_t transformedLength;
};

typedef struct SignData *sptr;

#define TYPE sptr
#include "../protection/VEC.h"
#undef TYPE

static const char const *globalMagic = "transformSign";
static const int globalTransformSignTag = 2;


struct SignTransformData
{
    const char const *magic;
    sptr_vec store;
    struct in_addr addr;
    rsa_private_key key;
    nnp_sequence_number_t seq;
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

/*
 * Create a handle to pass into the other functions.
 */
static TransformDataHandle dataHandleCreate(struct Transform *t)
{
    sptr ret;
    if(t && 
       t->transformData &&
       ((struct SignTransformData*)(t->transformData))->magic == 
       globalMagic)
    {
        ret = (sptr)malloc(sizeof(*ret));
        if(ret)
        {
            ret->magic = globalMagic;
            ret->std = (struct SignTransformData*)t->transformData;
            ret->raw = 0;
            ret->rawLength = 0;
            ret->transformed = 0;
            ret->transformedLength = 0;
            push_back_sptr(&ret->std->store, &ret);
        }
    }
    else
    {
        fprintf(stderr, "%s: null or invalid transform\n", __func__);
        ret = 0;
    }
    return (TransformDataHandle)ret;
}

/*
 * Set raw data to transform.
 *
 * This invalidates any data passed into "copy()"
 *
 * Returns zero on success.
 */
static int setRaw(
            TransformDataHandle handle, 
            void const *raw, 
            size_t rawLength)
{
    int ret;
    sptr h = (sptr)handle;
    if(h && h->magic == globalMagic)
    {
        size_t len = sizeof(globalTransformSignTag) + sizeof(h->std->addr) + rawLength;
        len += len + nnp_simple_additional_length_required(
                len, sizeof(globalTransformSignTag) + sizeof(h->std->addr));
        uint8_t *tmp = (uint8_t*)malloc(len);
        if(tmp)
        {
            int ntag = htonl(globalTransformSignTag);
            uint8_t *rdst = tmp;
            uint8_t *tdst = tmp + sizeof(ntag) + sizeof(h->std->addr) + rawLength;
            size_t protected_len;
            memcpy(rdst, &ntag, sizeof(ntag));
            memcpy(rdst + sizeof(ntag), &(h->std->addr), sizeof(h->std->addr));
            memcpy(rdst + sizeof(ntag) + sizeof(h->std->addr), raw, rawLength);
            if((ret = nnp_simple_protect(
                            &(h->std->key),
                            tdst,
                            rdst, 
                            sizeof(ntag) + sizeof(h->std->addr) + rawLength,
                            sizeof(ntag) + sizeof(h->std->addr),
                            h->std->seq,
                            &protected_len)) == 0)
            {
                // if raw is set, transformed is in same buffer.
                if(h->raw)
                {
                    free(h->raw);
                }
                else if(h->transformed)
                {
                    free(h->transformed);
                }
                h->raw = rdst;
                h->rawLength = rawLength + sizeof(ntag) + sizeof(h->std->addr);
                h->transformed = tdst;
                h->transformedLength = protected_len;
                h->std->seq = nnp_sequence_number_next(h->std->seq);
            }
            else
            {
                free(tmp);
                fprintf(stderr, "%s: Failed to sign. \"%s\" (%d)\n",
                        __func__, strerror(ret), ret);
            }
        }
        else
        {
            ret = ENOMEM;
            fprintf(stderr, "%s: Failed to allocate %u bytes\n",
                    __func__, rawLength);
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
 * Set pre-transformed data.
 *
 * This is how multiple signatures can get added to a payload. If
 * the transform is for multiple signatures and the "copyType"
 * parameter is TRANSFORM_COPY_AND_AMEND, then the copy function will
 * include an additional signature on the already transformed data.
 *
 * If the "copyType" parameter is set to TRANSFORM_COPY_AND_AMEND and
 * the transform cannot do amend, simply sets the transformed data
 * such that no transform calculation takes place.
 *
 * This invalidates any data passed into "setRaw()"
 *
 * Returns zero on success.
 */
static int copy(
            TransformDataHandle handle, 
            void const *transformed, 
            size_t transformedLength, 
            TransformCopyType copyType)
{
    /* Ignore copyType for sign transform */
    int ret;
    sptr h = (sptr)handle;
    if(h && h->magic == globalMagic)
    {
        void *tmp = malloc(sizeof(uint32_t) + transformedLength);
        if(tmp)
        {
            uint32_t tag = htonl(globalTransformSignTag);
            // if raw is set, transformed is in same buffer.
            if(h->raw)
            {
                free(h->raw);
            }
            else if(h->transformed)
            {
                free(h->transformed);
            }
            h->raw = 0;
            h->rawLength = 0;
            h->transformed = (uint8_t*)tmp;
            h->transformedLength = sizeof(tag) + transformedLength;
            memcpy(h->transformed, &tag, sizeof(tag));
            memcpy(h->transformed + sizeof(tag), 
                   transformed, transformedLength);
            ret = 0;
        }
        else
        {
            ret = ENOMEM;
            fprintf(stderr, "%s: Failed to allocate %u bytes\n",
                    __func__, transformedLength);
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
 * Return the raw data from "setRaw()".
 * 
 * Returns null on failure.
 */
static void const * raw(const TransformDataHandle handle)
{
    void const *ret;
    sptr h = (sptr)handle;
    if(h && h->magic == globalMagic)
    {
        if(h->raw)
        {
            // don't return tag or address
            ret = h->raw + sizeof(uint32_t) + sizeof(h->std->addr);
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
static size_t rawLength(const TransformDataHandle handle)
{
    size_t ret;
    sptr h = (sptr)handle;
    if(h && h->magic == globalMagic)
    {
        if(h->raw)
        {
            // don't include tag or address
            ret = h->rawLength - sizeof(uint32_t) - sizeof(h->std->addr); 
        }
        else
        {
            fprintf(stderr, "%s: Attempt to retrieve unset data\n", 
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
 * Return the transformed version of the data passed into "setRaw()"
 * or "copy()".
 *
 * Retuns null on failure.
 */
static void const * transformed(const TransformDataHandle handle)
{
    void const *ret;
    sptr h = (sptr)handle;
    if(h && h->magic == globalMagic)
    {
        if(h->transformed)
        {
            ret = h->transformed + sizeof(uint32_t); // don't send tag
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
} // transformed

/*
 * Return the length of the buffer returned by "transformed()" 
 *
 * Retuns zero on failure.
 */
static size_t transformedLength(const TransformDataHandle handle)
{
    size_t ret;
    sptr h = (sptr)handle;
    if(h && h->magic == globalMagic)
    {
        if(h->transformed)
        {
            // don't include tag
            ret = h->transformedLength - sizeof(uint32_t); 
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
} // transformedLength

/*
 * Cleans up resources owned by "handle". Note that the buffer
 * returned by "transformed()" is also cleaned up.
 */
static void dataHandleDestroy(TransformDataHandle handle)
{
    sptr h = (sptr)handle;
    if(h && h->magic == globalMagic)
    {
        sptr *check = find_sptr(&h->std->store, matchNptr, h);
        if(check)
        {
            remove_sptr(&h->std->store, check);
        }
        else
        {
            fprintf(stderr, "%s: Somehow got what looks to be a valid "
                    "handle but I can't find it in the store.\n",
                    __func__);
        }
        h->magic = 0;
        // if raw is set, transformed is in same buffer.
        if(h->raw)
        {
            free(h->raw);
        }
        else if(h->transformed)
        {
            free(h->transformed);
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
 * Cleans up resources owned by "transform". Only works
 * on an Transform like "u->transformDestroy(u)" - constructs
 * like "u1->transformDestroy(u2)" will likely fail. Do not use
 * the Transform after passing it into a destroyTransform()
 * function.
 */
static void transformDestroy(struct Transform *transform)
{
    if(transform)
    {
        struct SignTransformData *std = 
            ((struct SignTransformData*)transform->transformData);
        if(std && std->magic == globalMagic)
        {
            if(std->store.end != std->store.beg)
            {
                fprintf(stderr, "%s: cleaning %d outstanding handles "
                        "upon destroy\n",
                        __func__, std->store.end - std->store.beg);
            }
            while(std->store.end != std->store.beg)
            {
                dataHandleDestroy((TransformDataHandle)*(std->store.beg));
            }
            clean_sptr_vec(&std->store);
            std->magic = 0;
            free(transform);
        }
        else
        {
            fprintf(stderr, "%s: Attempt to destroy invalid transform\n",
                    __func__);
        }
    }
    return;
} // transformDestroy

/*********************************************************************/
/*********************************************************************/
/*********************************************************************/

/*
 * Reads the file and uses the first line found with the format:
 *
 * d.d.d.d <N>|<e>|<chk1>|<d>|<p>|<q>|<qP>|<dP>|<dQ>|<chk2><blah>
 *
 *   d.d.d.d - dotted decimal IPv4 address to use as an identifier.
 *   N       - modulus
 *   e       - public exponent (unused)
 *   chk1    - sha256-64 of N and e
 *   d       - private exponent
 *   p       - p factor of N
 *   q       - q factor of N
 *   qP      - 1/q mod p CRT param
 *   dP      - d mod (p - 1) CRT param
 *   dQ      - d mod (q - 1) CRT param
 *   chk2    - sha256-64 of N, d, p, q, dP, qP, dP, dQ
 *   blah    - can be anything that is not chk2
 *
 * The formats of "<N>" through "<chk2>" are as understood by
 * "string_to_fp_int()".
 *
 * The '#' character starts a comment which runs to the end of the line.
 */
Transform *transformSignCreate(char const *privateKeyFileName)
{
    Transform *ret;
    FILE *fp = fopen(privateKeyFileName, "r");
    if(fp)
    {
        for(;;)
        {
            char *line =  mallocreadnextnonblank(fp);
            struct in_addr addr;
            char const *key;
            if(line)
            {
                if((key = parseIPv4(&addr, line)) != 0 &&
                        (key = firstNonSpaceOf(key)) != 0)
                {
                    int res;
                    char *end = strchr(key, 0);
                    rsa_private_key priv;
                    if((res = loadRSAPrivateKey(&priv, key, end)) == 0)
                    {
                        // append the transform data to the end of the 
                        // Transform structure.
                        ret = (Transform*)malloc(
                                sizeof(*ret) + 
                                sizeof(struct SignTransformData));
                        if(ret)
                        {
                            struct SignTransformData *std = 
                                (struct SignTransformData*)(ret + 1);
                            ret->transformData = std;
                            ret->tag = globalTransformSignTag;
                            ret->dataHandleCreate = dataHandleCreate;
                            ret->setRaw  = setRaw;
                            ret->copy = copy;
                            ret->raw = raw;
                            ret->rawLength = rawLength;
                            ret->transformed = transformed;
                            ret->transformedLength = transformedLength;
                            ret->dataHandleDestroy = dataHandleDestroy;
                            ret->transformDestroy = transformDestroy;
                            std->magic = globalMagic;
                            std->key = priv;
                            std->seq = htonl(time(0) << 5); // 32/second max
                            std->store.beg = 0;
                            std->store.end = 0;
                            std->store.eos = 0;
                            std->addr = addr;
                        }
                        else
                        {
                            fprintf(stderr, 
                                    "%s: Failed allocate %d bytes\n",
                                    __func__,
                                    sizeof(*ret) + 
                                    sizeof(struct SignTransformData));
                        }
                        free(line);
                        break;
                    }
                    fprintf(stderr, "%s: Error reading \"%s\" as private key, "
                            "will keep looking. \"%s\" (%d)\n", 
                            __func__, line, strerror(res), res);
                }
                else
                {
                    fprintf(stderr, "%s: Error reading \"%s\" as IP address/private key, "
                            "will keep looking.\n", 
                            __func__, line);
                }
                free(line);
            }
            else
            {
                ret = 0;
                fprintf(stderr, "%s: Private key not found in \"%s\"\n",
                        __func__, privateKeyFileName);
                break;
            }
        }
        fclose(fp);
    }
    else
    {
        ret = 0;
        fprintf(stderr, "%s: Failed to open \"%s\" for reading. "
                "\"%s\" (%d)\n", 
                __func__, privateKeyFileName, strerror(errno), errno);
    }
    return ret;
} // transformSignCreate
