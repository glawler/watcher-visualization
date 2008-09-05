/*
 * bufferPair.c - manage a pair of buffers
 *
 * Copyright (C) 2007  Sparta Inc.  Written by the NIP group, SRD, ISSO
 */

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bufferPair.h"
#include "transDuo.h"

/*
 * Holds the buffer pair.
 *
 * This structure is largely pre-marshaled. The marshaled data is from
 * "firstLength" on to the end of "buf"
 */
struct BufferPair
{
    char const * magic;
    uint32_t firstLength; // network byte order
    uint32_t secondLength; // network byte order
    uint8_t buf[0];
};

/*
 * Offset in bytes from start of a BufferPair where the marshaled data
 * starts.
 */
static size_t const globalMarshalOffset = 
    ((size_t) &((struct BufferPair *)0)->firstLength);

/*
 * How we know if it is a valid BufferPair
 */
static char const * const globalMagic = "BufferPair";

/*
 * Create a BufferPair from a TransDuo
 * Puts transformed in bufferPairFirst(), raw in bufferPairSecond()
 */
struct BufferPair *bufferPairFromTransDuo(struct TransDuo *td)
{
    struct BufferPair *ret = bufferPairFromBuffers(
        transDuoTransformedGet(td),
        transDuoTransformedLengthGet(td),
        transDuoRawGet(td),
        transDuoRawLengthGet(td));
    if(!ret)
    {
        fprintf(stderr, 
                "%s: Failed to create a BufferPair from a TransDuo\n",
                __func__);
    }
    return ret;
} /* bufferPairFromTransDuo */

/*
 * Create a BufferPair from a buffer created by bufferPairMarshal()
 */
struct BufferPair *bufferPairFromMarshaled(
        void const *buf, size_t buflen)
{
    struct BufferPair *ret;
    if(buflen + globalMarshalOffset >= sizeof(struct BufferPair))
    {
        struct BufferPair const *tmp = (struct BufferPair const *)
            (((uint8_t const *)buf) - globalMarshalOffset);
        size_t minLength = 
            (sizeof(*tmp) - globalMarshalOffset) + 
            ntohl(tmp->firstLength) + ntohl(tmp->secondLength);
        if(minLength <= buflen)
        {
            ret = (struct BufferPair *)
                malloc(globalMarshalOffset + minLength);
            if(ret)
            {
                ret->magic = globalMagic;
                memcpy(((uint8_t *)ret) + globalMarshalOffset,
                        buf, minLength);
            }
            else
            {
                fprintf(stderr, "%s: Failed to allocate %d bytes\n", 
                        __func__, globalMarshalOffset + minLength);
            }
        }
        else
        {
            // buflen too small
            fprintf(stderr, "%s: require %u bytes to unmarshal but got "
                    "only %u bytes\n", __func__, minLength, buflen);
            ret = 0;
        }
    }
    else
    {
        // buflen not even long enough for lengths.
        fprintf(stderr, "%s: require %u bytes to start unmarshaling "
                "but got only %u bytes\n", __func__, 
                sizeof(struct BufferPair) - globalMarshalOffset,
                buflen);
        ret = 0;
    }
    return ret;
} /* bufferPairFromMarshalled */

/*
 * Create a BufferPair from two buffers.
 */
struct BufferPair *bufferPairFromBuffers(
        void const *first, size_t firstLength,
        void const *second, size_t secondLength)
{
    struct BufferPair *ret;
    size_t len = sizeof(*ret) + firstLength + secondLength;
    ret = malloc(len);
    if(ret)
    {
        ret->magic = globalMagic;
        ret->firstLength = htonl(firstLength);
        ret->secondLength = htonl(secondLength);
        memcpy(ret->buf, first, firstLength);
        memcpy(ret->buf + firstLength, second, secondLength);
    }
    else
    {
        fprintf(stderr, "%s: Failed to allocate %d bytes\n", 
                __func__, len);
    }
    return ret;
} /* bufferPairFromBuffers */

/*
 * Destroy a BufferPair
 */
void bufferPairDestroy(struct BufferPair *toDestroy)
{
    if(toDestroy && toDestroy->magic == globalMagic)
    {
        toDestroy->magic = 0;
        free(toDestroy);
    }
    else
    {
        fprintf(stderr, "%s: invalid BufferPair\n", __func__);
    }
    return;
} /* bufferPairDestroy */

/*
 * Turn a buffer pair into a single buffer that can be separated later.
 */
void const *bufferPairMarshal(struct BufferPair const *toMarshal)
{
    void const *ret;
    if(toMarshal && toMarshal->magic == globalMagic)
    {
        ret = ((uint8_t const*)toMarshal) + globalMarshalOffset;
    }
    else
    {
        fprintf(stderr, "%s: invalid BufferPair\n", __func__);
        ret = 0;
    }
    return ret;
} /* bufferPairMarshal */

/*
 * Make a copy of what bufferPairMarshal() returns.
 *
 * It is the caller's responsibility to free() the returned value.
 */
void *bufferPairMarshalDup(struct BufferPair const *toMarshal)
{
    void *ret;
    if(toMarshal && toMarshal->magic == globalMagic)
    {
        size_t len = 
                sizeof(*toMarshal) - 
                globalMarshalOffset + 
                ntohl(toMarshal->firstLength) + 
                ntohl(toMarshal->secondLength);
        ret = malloc(len);
        if(ret)
        {
            memcpy(ret, 
                   ((uint8_t const*)toMarshal) + globalMarshalOffset, 
                   len);
        }
    }
    else
    {
        ret = 0;
        fprintf(stderr, "%s: invalid BufferPair\n", __func__);
    }
    return ret;
} /* bufferPairMarshalDup */

/*
 * The length of the buffer returned by bufferPairMarshal().
 */
size_t bufferPairMarshalLength(struct BufferPair const *toMarshal)
{
    size_t ret;
    if(toMarshal && toMarshal->magic == globalMagic)
    {
        ret = (sizeof(*toMarshal) - globalMarshalOffset) +
            ntohl(toMarshal->firstLength) + ntohl(toMarshal->secondLength);
    }
    else
    {
        fprintf(stderr, "%s: invalid BufferPair\n", __func__);
        ret = 0;
    }
    return ret;
} /* bufferPairMarshalLength */

/*
 * Return the first buffer in the BufferPair
 */
void const *bufferPairFirst(struct BufferPair const *toGet)
{
    void const *ret;
    if(toGet && toGet->magic == globalMagic)
    {
        ret = toGet->buf;
    }
    else
    {
        fprintf(stderr, "%s: invalid BufferPair\n", __func__);
        ret = 0;
    }
    return ret;
} /* bufferPairFirst */

/*
 * Return a copy of the first buffer in the BufferPair
 *
 * It is the caller's responsibility to free() the returned value.
 */
void *bufferPairFirstDup(struct BufferPair const *toGet)
{
    void *ret;
    if(toGet && toGet->magic == globalMagic)
    {
        ret = malloc(ntohl(toGet->firstLength));
        if(ret)
        {
            memcpy(ret, toGet->buf, ntohl(toGet->firstLength));
        }
        else
        {
            fprintf(stderr, "%s: Failed allocate %d bytes\n", __func__,
                    ntohl(toGet->firstLength));
        }
    }
    else
    {
        fprintf(stderr, "%s: invalid BufferPair\n", __func__);
        ret = 0;
    }
    return ret;
} /* bufferPairFirstDup */

/*
 * The length of the buffer returned by bufferPairFirst().
 */
size_t bufferPairFirstLength(struct BufferPair const *toGet)
{
    size_t ret;
    if(toGet && toGet->magic == globalMagic) 
    {
        ret = ntohl(toGet->firstLength);
    }
    else
    {
        fprintf(stderr, "%s: invalid BufferPair\n", __func__);
        ret = 0;
    }
    return ret;
} /* bufferPairFirstLength */

/*
 * Return the second buffer in the BufferPair
 */
void const *bufferPairSecond(struct BufferPair const *toGet)
{
    void const *ret;
    if(toGet && toGet->magic == globalMagic)
    {
        ret = toGet->buf + ntohl(toGet->firstLength);
    }
    else
    {
        fprintf(stderr, "%s: invalid BufferPair\n", __func__);
        ret = 0;
    }
    return ret;
} /* bufferPairSecond */

/*
 * Return a copy of the second buffer in the BufferPair
 *
 * It is the caller's responsibility to free() the returned value.
 */
void *bufferPairSecondDup(struct BufferPair const *toGet)
{
    void *ret;
    if(toGet && toGet->magic == globalMagic)
    {
        ret = malloc(ntohl(toGet->secondLength));
        if(ret)
        {
            memcpy(ret, toGet->buf + ntohl(toGet->firstLength),
                    ntohl(toGet->secondLength));
        }
        else
        {
            fprintf(stderr, "%s: Failed allocate %d bytes\n", __func__,
                    ntohl(toGet->secondLength));
        }
    }
    else
    {
        fprintf(stderr, "%s: invalid BufferPair\n", __func__);
        ret = 0;
    }
    return ret;
} /* bufferPairSecondDup */

/*
 * The length of the buffer returned by bufferPairSecond()
 */
size_t bufferPairSecondLength(struct BufferPair const *toGet)
{
    size_t ret;
    if(toGet && toGet->magic == globalMagic)
    {
        ret = ntohl(toGet->secondLength);
    }
    else
    {
        fprintf(stderr, "%s: invalid BufferPair\n", __func__);
        ret = 0;
    }
    return ret;
} /* bufferPairSecondLength */
