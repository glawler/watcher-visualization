/*
 * transformNull.c - perform a null transformation 
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "transformNull.h"

struct NullData
{
    const char const *magic;
    struct NullTransformData *ntd;
    void *data;
    size_t dataLength;
};

typedef struct NullData *nptr;

#define TYPE nptr
#include "../protection/VEC.h"
#undef TYPE

static const char const *globalMagic = "transformNull";

struct NullTransformData
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
static TransformDataHandle dataHandleCreate(struct Transform *t)
{
    nptr ret;
    if(t && 
       t->transformData &&
       ((struct NullTransformData*)(t->transformData))->magic == 
       globalMagic)
    {
        ret = (nptr)malloc(sizeof(*ret));
        if(ret)
        {
            ret->magic = globalMagic;
            ret->ntd = (struct NullTransformData*)t->transformData;
            ret->data = 0;
            ret->dataLength = 0;
            push_back_nptr(&ret->ntd->store, &ret);
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
    nptr h = (nptr)handle;
    if(h && h->magic == globalMagic)
    {
        void *tmp = malloc(rawLength);
        if(tmp)
        {
            if(h->data)
            {
                free(h->data);
            }
            h->data = tmp;
            h->dataLength = rawLength;
            memcpy(h->data, raw, rawLength);
            ret = 0;
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
    /* Ignore copyType for null transform */
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
 * Return the raw data from "setRaw()".
 * 
 * Returns null on failure.
 */
static void const * raw(const TransformDataHandle handle)
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
static size_t rawLength(const TransformDataHandle handle)
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
 * Return the transformed version of the data passed into "setRaw()"
 * or "copy()".
 *
 * Retuns null on failure.
 */
static void const * transformed(const TransformDataHandle handle)
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
static size_t transformedLength(const TransformDataHandle handle)
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
 * returned by "transformed()" is also cleaned up.
 */
static void dataHandleDestroy(TransformDataHandle handle)
{
    nptr h = (nptr)handle;
    if(h && h->magic == globalMagic)
    {
        nptr *check = find_nptr(&h->ntd->store, matchNptr, h);
        if(check)
        {
            remove_nptr(&h->ntd->store, check);
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
        struct NullTransformData *ntd = 
            ((struct NullTransformData*)transform->transformData);
        if(ntd && ntd->magic == globalMagic)
        {
            if(ntd->store.end != ntd->store.beg)
            {
                fprintf(stderr, "%s: cleaning %d outstanding handles "
                        "upon destroy\n",
                        __func__, ntd->store.end - ntd->store.beg);
            }
            while(ntd->store.end != ntd->store.beg)
            {
                dataHandleDestroy((TransformDataHandle)*(ntd->store.beg));
            }
            clean_nptr_vec(&ntd->store);
            ntd->magic = 0;
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

const int transformNullTag = 1;

Transform *transformNullCreate(void)
{
    // append the transform data to the end of the Transform structure.
    Transform *ret = 
        (Transform*)malloc(sizeof(*ret) + sizeof(struct NullTransformData));
    if(ret)
    {
        struct NullTransformData *ntd = 
            (struct NullTransformData*)(ret + 1);
        ret->transformData = ntd;
        ret->tag = transformNullTag;
        ret->dataHandleCreate = dataHandleCreate;
        ret->setRaw  = setRaw;
        ret->copy = copy;
        ret->raw = raw;
        ret->rawLength = rawLength;
        ret->transformed = transformed;
        ret->transformedLength = transformedLength;
        ret->dataHandleDestroy = dataHandleDestroy;
        ret->transformDestroy = transformDestroy;
        ntd->magic = globalMagic;
        ntd->store.beg = 0;
        ntd->store.end = 0;
        ntd->store.eos = 0;
    }
    else
    {
        fprintf(stderr, "%s: Failed allocate %d bytes\n",
                __func__,
               sizeof(*ret) + sizeof(struct NullTransformData));
    }
    return ret;
}
