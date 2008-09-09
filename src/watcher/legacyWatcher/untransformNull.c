/*
 * untransformNull.c - perform a null untransformation 
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "untransformNull.h"

struct NullData
{
    const char const *magic;
    struct NullUntransformData *nud;
    void *data;
    size_t dataLength;
};

typedef struct NullData *nptr;

#define TYPE nptr
#include "../protection/VEC.h"
#undef TYPE

static const char const *globalMagic = "untransformNull";

struct NullUntransformData
{
    const char const *magic;
    nptr_vec store;
};

/*********************************************************************/
/*********************************************************************/
/*********************************************************************/

static int matchNptr(nptr const *ptr, void *check)
{
    return *ptr == check;
}

/*
 * Create a handle to pass into the other functions.
 */
static UntransformDataHandle dataHandleCreate(struct Untransform *u)
{
    nptr ret;
    if(u && 
       u->untransformData &&
       ((struct NullUntransformData*)(u->untransformData))->magic == 
       globalMagic)
    {
        ret = (nptr)malloc(sizeof(*ret));
        if(ret)
        {
            ret->magic = globalMagic;
            ret->nud = (struct NullUntransformData*)u->untransformData;
            ret->data = 0;
            ret->dataLength = 0;
            push_back_nptr(&ret->nud->store, &ret);
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
    nptr h = (nptr)handle;
    if(h && h->magic == globalMagic)
    {
        void *tmp = malloc(transformedLength);
        if(tmp)
        {
            if(h->data)
            {
                free(h->data);
            }
            h->data = tmp;
            h->dataLength = transformedLength;
            memcpy(h->data, transformed, transformedLength);
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
 * Return the raw data from untransforming the data passed into
 * "setTransformed()"
 * 
 * Returns null on failure.
 */
static void const * raw(const UntransformDataHandle handle)
{
    void const *ret;
    nptr h = (nptr)handle;
    if(h && h->magic == globalMagic)
    {
        ret = h->data;
        if(ret == 0)
        {
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
    nptr h = (nptr)handle;
    if(h && h->magic == globalMagic)
    {
        ret = h->dataLength;
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
    nptr h = (nptr)handle;
    if(h && h->magic == globalMagic)
    {
        ret = h->data;
        if(ret == 0)
        {
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
static size_t transformedLength(const UntransformDataHandle handle)
{
    size_t ret;
    nptr h = (nptr)handle;
    if(h && h->magic == globalMagic)
    {
        ret = h->dataLength;
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
    nptr h = (nptr)handle;
    if(h && h->magic == globalMagic)
    {
        nptr *check = find_nptr(&h->nud->store, matchNptr, h);
        if(check)
        {
            remove_nptr(&h->nud->store, check);
        }
        else
        {
            fprintf(stderr, "%s: Somehow got what looks to be a valid "
                    "handle but I can't find it in the store.\n",
                    __func__);
        }
        h->magic = 0;
        if(h->data)
        {
            free(h->data);
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
        struct NullUntransformData *nud = 
            ((struct NullUntransformData*)untransform->untransformData);
        if(nud && nud->magic == globalMagic)
        {
            if(nud->store.end != nud->store.beg)
            {
                fprintf(stderr, "%s: cleaning %d outstanding handles "
                        "upon destroy\n",
                        __func__, nud->store.end - nud->store.beg);
            }
            while(nud->store.end != nud->store.beg)
            {
                dataHandleDestroy((UntransformDataHandle)*(nud->store.beg));
            }
            clean_nptr_vec(&nud->store);
            nud->magic = 0;
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

const int untransformNullTag = 1;

Untransform *untransformNullCreate(void)
{
    // append the untransform data to the end of the Untransform structure.
    Untransform *ret = 
        (Untransform*)malloc(sizeof(*ret) + sizeof(struct NullUntransformData));
    if(ret)
    {
        struct NullUntransformData *nud = 
            (struct NullUntransformData*)(ret + 1);
        ret->untransformData = nud;
        ret->tag = untransformNullTag;
        ret->dataHandleCreate = dataHandleCreate;
        ret->setTransformed  = setTransformed;
        ret->raw = raw;
        ret->rawLength = rawLength;
        ret->transformed = transformed;
        ret->transformedLength = transformedLength;
        ret->dataHandleDestroy = dataHandleDestroy;
        ret->untransformDestroy = untransformDestroy;
        nud->magic = globalMagic;
        nud->store.beg = 0;
        nud->store.end = 0;
        nud->store.eos = 0;
    }
    else
    {
        fprintf(stderr, "%s: Failed allocate %d bytes\n",
                __func__,
               sizeof(*ret) + sizeof(struct NullUntransformData));
    }
    return ret;
}
