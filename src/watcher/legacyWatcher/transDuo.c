/*
 * transDuo.c - work with raw/transformed buffer pairs.
 *
 * Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "transDuo.h"
#include "sequence_number.h"

#define UINT32_TO_BUFFER(u, b) \
    do { \
        ((uint8_t *)(b))[0] = ((uint8_t)((u) >> 24)); \
        ((uint8_t *)(b))[1] = ((uint8_t)((u) >> 16)); \
        ((uint8_t *)(b))[2] = ((uint8_t)((u) >> 8)); \
        ((uint8_t *)(b))[3] = ((uint8_t)(u)); } while(0)

#define UINT32_FROM_BUFFER(b) \
    (((((uint8_t const *)(b))[0]) << 24) | \
     ((((uint8_t const *)(b))[1]) << 16) | \
     ((((uint8_t const *)(b))[2]) << 8) | \
     (((uint8_t const *)(b))[3]))

typedef enum
{
    TRANSFORM,
    UNTRANSFORM
} Direction;

struct TransDuo
{
    const char const *magic;
    struct TransDuoManager *manager;
    Direction direction;
    union
    {
        struct
        {
            Transform *x;
            TransformDataHandle h;
        } t;
        struct
        {
            Untransform *x;
            UntransformDataHandle h;
        } u;
    } uort;
    uint8_t *buffer;
    size_t bufferLength;
};

typedef Untransform *uptr;
typedef struct TransDuo *dptr;
#define TYPE uptr
#include "../protection/VEC.h"
#undef TYPE 
#define TYPE dptr
#include "../protection/VEC.h"
#undef TYPE 

struct TransDuoManager
{
    const char const *magic;
    uptr_vec ustore;
    dptr_vec tdstore;
};

typedef struct TransDuoManager *mptr;
#define TYPE mptr
#include "../protection/VEC.h"
#undef TYPE 

static const char const *globalMagic = "TransDuo";
static const char const *globalTransDuoManagerMagic = "TransDuo Handle";
static mptr_vec globalManager = { 0, 0, 0 };

static int matchesTag(uptr const *ptr, void *matchesDat)
{
    return (*ptr)->tag == *((uint32_t*)matchesDat);
}

static int matchesTransDuo(dptr const *ptr, void *matchesDat)
{
    return *ptr == matchesDat;
}

static int matchesTransDuoManager(mptr const *ptr, void *matchesDat)
{
    return *ptr == matchesDat;
}

/*
 * Expects "m->ustore" to not hold an untransform with the same tag as "u".
 */
static void registerNewUntransform(struct TransDuoManager *m, Untransform *u)
{
    int res;
    if((res = push_back_uptr(&(m->ustore), &u)) == 0)
    {
        // success
    }
    else
    {
        u->untransformDestroy(u);
        fprintf(stderr, "%s: Failed to store new untransform %d. "
                "\"%s\" (%d)\n", 
                __func__, u->tag, strerror(res), res);
    }
    return;
} // registerNewUntransform

/*
 * Expects "td" and "buffer" to be non-null.
 */
static void bufferInitFromData(
        struct TransDuo *td,
        uint32_t tag,
        void const *buffer,
        size_t bufferLength)
{
    size_t l = sizeof(tag) + bufferLength;
    td->buffer = (uint8_t*)malloc(l);
    if(td->buffer)
    {
        UINT32_TO_BUFFER(tag, td->buffer);
        memcpy(td->buffer + sizeof(tag), buffer, bufferLength);
        td->bufferLength = l;
    }
    else
    {
        fprintf(stderr, 
                "%s: Failed to allocate %d byte marshall buffer\n", 
                __func__, l);
    }
    return;
} // bufferInitFromData

/*
 * 
 */
static void bufferInitFromTransform(struct TransDuo *td)
{
    if(td->uort.t.x && td->uort.t.h)
    {
        void const *buffer = td->uort.t.x->transformed(td->uort.t.h);
        if(buffer)
        {
            bufferInitFromData(
                    td, 
                    td->uort.t.x->tag,
                    buffer,
                    td->uort.t.x->transformedLength(td->uort.t.h));
        }
    }
    else
    {
        fprintf(stderr, "%s: No transform%s\n", 
                __func__, td->uort.t.x == 0 ? "" : " handle");
    }
    return;
} // bufferInitFromTransform

/*
 * 
 */
static void bufferInitFromUntransform(struct TransDuo *td)
{
    if(td->uort.u.x && td->uort.u.h)
    {
        void const *buffer = td->uort.u.x->transformed(td->uort.u.h);
        if(buffer)
        {
            bufferInitFromData(
                    td, 
                    td->uort.u.x->tag,
                    buffer,
                    td->uort.u.x->transformedLength(td->uort.u.h));
        }
    }
    else
    {
        fprintf(stderr, "%s: No untransform%s\n", 
                __func__, td->uort.u.x == 0 ? "" : " handle");
    }
    return;
} // bufferInitFromUntransform

/*
 * Fill in "td->buffer" and "td->bufferLength"
 */
static void bufferInit(struct TransDuo *td)
{
    if(td->direction == TRANSFORM)
    {
        bufferInitFromTransform(td);
    }
    else
    {
        bufferInitFromUntransform(td);
    }
    return;
}

/*
 * Destroy and zeroize the transDuo transform handle.
 */
static void transformHandleDestroy(struct TransDuo *td)
{
    if(td->direction == TRANSFORM)
    {
        if(td->uort.t.h)
        {
            if(td->uort.t.x)
            {
                td->uort.t.x->dataHandleDestroy(td->uort.t.h);
            }
            else
            {
                fprintf(stderr, "%s: Expected to have a transform "
                        "to destroy the handle with\n", __func__);
            }
            td->uort.t.h = 0;
        }
    }
    else
    {
        if(td->uort.u.h)
        {
            if(td->uort.u.x)
            {
                td->uort.u.x->dataHandleDestroy(td->uort.u.h);
            }
            else
            {
                fprintf(stderr, "%s: Expected to have an "
                        "untransform to destroy the handle with\n",
                        __func__);
            }
            td->uort.u.h = 0;
        }
    }
    return;
} // transformHandleDestroy

/*
 * Get the transformed bytes (unmarshaled) from the given transDuo.
 * 
 * Returns zero on success.
 */
static int loadTransformed(
        struct TransDuo *td, 
        void const **transformed_ret,
        size_t *transformedLength_ret)
{
    int ret;
    if(td->direction == TRANSFORM)
    {
        if(td->uort.t.h)
        {
            if(td->uort.t.x)
            {
                *transformed_ret = td->uort.t.x->transformed(td->uort.t.h);
                *transformedLength_ret = 
                    td->uort.t.x->transformedLength(td->uort.t.h);
                ret = 0;
            }
            else
            {
                fprintf(stderr, "%s: Have a transform data handle, "
                        "expected to have a transform\n", __func__);
                ret = EINVAL;
            }
        }
        else
        {
            fprintf(stderr, "%s: null transform data handle\n",
                    __func__);
            ret = EINVAL;
        }
    }
    else
    {
        if(td->uort.u.h)
        {
            if(td->uort.u.x)
            {
                *transformed_ret = td->uort.u.x->transformed(td->uort.u.h);
                *transformedLength_ret = 
                    td->uort.u.x->transformedLength(td->uort.u.h);
                ret = 0;
            }
            else
            {
                fprintf(stderr, "%s: Have an untransform data handle, "
                        "expected to have an untransform\n", __func__);
                ret = EINVAL;
            }
        }
        else
        {
            fprintf(stderr, "%s: null untransform data handle\n",
                    __func__);
            ret = EINVAL;
        }
    }
    return ret;
} // loadTransformed

/*********************************************************************/
/*********************************************************************/
/*********************************************************************/

/*
 * Perform any transDuo initialization processing.
 */
void transDuoInit(void)
{
    /* we don't want replay detection -- clients will handle it */
    nnp_set_replay_check_enabled(0);
} // transDuoInit

/*
 * Will clean up any transDuo static data.
 */
void transDuoFini(void)
{
    if(globalManager.end != globalManager.beg)
    {
        fprintf(stderr, "%s: %d TransDuoManager value%s "
                "created but not destroyed\n", __func__,
                globalManager.end - globalManager.beg,
                (globalManager.end - globalManager.beg) == 1 ? 
                "" : "s");
        while(globalManager.end != globalManager.beg)
        {
            transDuoManagerDestroy(*(globalManager.end - 1));
        }
    }
    return;
} // transDuoFini

/*
 * Create a handle to a TransDuo manager
 */
struct TransDuoManager *transDuoManagerCreate(void)
{
    struct TransDuoManager *ret = (struct TransDuoManager *)malloc(sizeof(*ret));
    if(ret)
    {
        ret->magic = globalTransDuoManagerMagic;
        ret->ustore.beg = 0;
        ret->ustore.end = 0;
        ret->ustore.eos = 0;
        ret->tdstore.beg = 0;
        ret->tdstore.end = 0;
        ret->tdstore.eos = 0;
        push_back_mptr(&globalManager, &ret);
    }
    return ret;
} // transDuoManagerCreate

/*
 * Destroy a TransDuo manager handle created in
 * "transDuoManagerCreate()"
 */
void transDuoManagerDestroy(struct TransDuoManager *h)
{
    if(h && h->magic == globalTransDuoManagerMagic)
    {
        while(h->tdstore.beg != h->tdstore.end)
        {
            dptr *rem = h->tdstore.end - 1;
            transDuoDestroy(*rem); // removes it from the store
        }
        clean_dptr_vec(&h->tdstore);
        while(h->ustore.beg != h->ustore.end)
        {
            uptr *rem = h->ustore.end - 1;
            Untransform *tmp = *rem;
            if(remove_uptr(&h->ustore, rem))
            {
                tmp->untransformDestroy(tmp);
            }
        }
        clean_uptr_vec(&h->ustore);
        mptr *check = 
            find_mptr(&globalManager, matchesTransDuoManager, h);
        if(check)
        {
            remove_mptr(&globalManager, check);
        }
        else
        {
            fprintf(stderr, "%s: Lost track of a manager that "
                    "just got deleted\n", __func__);
        }
        free(h);
    }
    return;
} // transDuoManagerDestroy

/*
 * Register an untransform with the transDuo code so when
 * "transDuoFromBuffer()" gets called, the parsing can look up which
 * untransform to use to parse it.
 * 
 * Takes ownership of "u" and will free it when it is done with it.
 * 
 * If registering an untransform with the same tag as an already
 * registered untransform, the existing untransform will get replaced.
 */
void transDuoUntransformRegister(struct TransDuoManager *m, Untransform *u)
{
    if(m && m->magic == globalTransDuoManagerMagic)
    {
        uptr *check = find_uptr(&m->ustore, matchesTag, &(u->tag));
        if(check)
        {
            if(*check != u)
            {
                Untransform *urem = *check;
                dptr *cur = m->tdstore.end;
                while(cur != m->tdstore.beg)
                {
                    --cur;
                    if((*cur)->direction == UNTRANSFORM &&
                            (*cur)->uort.u.x == urem)
                    {
                        transDuoDestroy(*cur);
                    }
                }
                remove_uptr(&m->ustore, check);
                urem->untransformDestroy(urem);
                registerNewUntransform(m, u);
            }
            else
            {
                fprintf(stderr, "%s: Attempt to register untransform %d twice\n",
                        __func__, u->tag);
            }
        }
        else
        {
            registerNewUntransform(m, u);
        }
    }
    else
    {
        fprintf(stderr, 
                "%s: Attempt to register an Untransform on %s "
                "TransDuoManager\n", 
                __func__, m ? "an invalid" : "a null");
    }
    return;
} // transDuoUntransformRegister

/***********************
 * TransDuo functions
 ***********************/

/*
 * Create an empty transDuo handle.
 *
 * Returns 0 on failure to allocate.
 */
struct TransDuo *transDuoCreate(struct TransDuoManager *m)
{
    struct TransDuo *ret;
    if(m && m->magic == globalTransDuoManagerMagic)
    {
        ret = (struct TransDuo*)malloc(sizeof(*ret));
        if(ret)
        {
            ret->magic = globalMagic;
            ret->manager = m;
            ret->direction = TRANSFORM;
            ret->uort.t.x = 0;
            ret->uort.t.h = 0;
            ret->buffer = 0;
            ret->bufferLength = 0;
            push_back_dptr(&(m->tdstore), &ret);
        }
    }
    else
    {
        ret = 0;
        fprintf(stderr, 
                "%s: Attempt to register an Untransform on %s "
                "TransDuoManager\n", 
                __func__, m ? "an invalid" : "a null");
    }
    return ret;
} // transDuoCreate

/*
 * Create a transDuo from a buffer. The buffer must be formatted as 
 * the result from "transDuoBufferGet()"
 */
struct TransDuo *transDuoFromBuffer(
        struct TransDuoManager *m,
        void const *buffer, 
        size_t bufferLength)
{
    struct TransDuo *ret;
    if(m && m->magic == globalTransDuoManagerMagic)
    {
        uint32_t tag;
        if(buffer && bufferLength > sizeof(tag))
        {
            uptr *tmp;
            tag = UINT32_FROM_BUFFER(buffer);
            tmp = find_uptr(&(m->ustore), matchesTag, &tag);
            if(tmp)
            {
                Untransform *u = *tmp;
                UntransformDataHandle h = u->dataHandleCreate(u);
                if(h)
                {
                    int res;
                    buffer = ((uint8_t const *)buffer) + sizeof(tag);
                    bufferLength -= sizeof(tag);
                    if((res = u->setTransformed(h, buffer, bufferLength)) == 0)
                    {
                        ret = transDuoCreate(m);
                        if(ret)
                        {
                            ret->direction = UNTRANSFORM;
                            ret->uort.u.x = u;
                            ret->uort.u.h = h;
                        }
                        else
                        {
                            fprintf(stderr, "%s: Failed to create a transDuo handle\n",
                                    __func__);
                            u->dataHandleDestroy(h);
                            ret = 0;
                        }
                    }
                    else
                    {
                        fprintf(stderr, "%s: Failed to untransform. \"%s\" (%d)\n",
                                __func__, strerror(res), res);
                        u->dataHandleDestroy(h);
                        ret = 0;
                    }
                }
                else
                {
                    fprintf(stderr, "%s: failed to create untransform handle\n",
                            __func__);
                    ret = 0;
                }
            }
            else
            {
                fprintf(stderr, "%s: Failed to find untransform for tag %d\n",
                        __func__, tag);
                ret = 0;
            }
        }
        else
        {
            fprintf(stderr, "%s: null or short buffer cannot be parsed\n",
                    __func__);
            ret = 0;
        }
    }
    else
    {
        ret = 0;
        fprintf(stderr, 
                "%s: Attempt to register an Untransform on %s "
                "TransDuoManager\n", 
                __func__, m ? "an invalid" : "a null");
    }
    return ret;
} // transDuoFromBuffer

/*
 * Get the buffer that is the raw buffer transformed and
 * marshaled into a byte array. That is formatted so
 * "transDuoFromBuffer()" can parse it.
 */
void const *transDuoBufferGet(struct TransDuo *transDuo)
{
    void const *ret;
    if(transDuo && transDuo->magic == globalMagic)
    {
        if(transDuo->buffer != 0 ||
                (bufferInit(transDuo), transDuo->buffer != 0))
        {
            ret = transDuo->buffer;
        }
        else
        {
            fprintf(stderr, 
                    "%s: Failed to marshall transDuo information\n",
                    __func__);
            ret = 0;
        }
    }
    else
    {
        ret = 0;
        fprintf(stderr, "%s: null or invalid transDuo handle\n",
                __func__);
    }
    return ret;
} // transDuoBufferGet

/*
 * Allocate and return a copy of the buffer returned in
 * "transDuoBufferGet()".
 */
void *transDuoBufferDup(struct TransDuo *transDuo)
{
    void *ret;
    if(transDuo && transDuo->magic == globalMagic)
    {
        if(transDuo->buffer != 0 ||
                (bufferInit(transDuo), transDuo->buffer != 0))
        {
            ret = malloc(transDuo->bufferLength);
            if(ret)
            {
                memcpy(ret, transDuo->buffer, transDuo->bufferLength);
            }
            else
            {
                fprintf(stderr, "%s: Failed to allocate %d byte buffer "
                        "for copy of marshaled transDuo\n", __func__,
                        transDuo->bufferLength);
            }
        }
        else
        {
            fprintf(stderr, 
                    "%s: Failed to marshall transDuo information\n",
                    __func__);
            ret = 0;
        }
    }
    else
    {
        ret = 0;
        fprintf(stderr, "%s: null or invalid transDuo handle\n",
                __func__);
    }
    return ret;
} // transDuoBufferDup

/*
 * Get the length of the buffer returned by "transDuoGetBuffer()"
 */
size_t transDuoBufferLengthGet(struct TransDuo *transDuo)
{
    size_t ret;
    if(transDuo && transDuo->magic == globalMagic)
    {
        if(transDuo->buffer != 0 ||
                (bufferInit(transDuo), transDuo->buffer != 0))
        {
            ret = transDuo->bufferLength;
        }
        else
        {
            fprintf(stderr, 
                    "%s: Failed to marshall transDuo information\n",
                    __func__);
            ret = 0;
        }
    }
    else
    {
        ret = 0;
        fprintf(stderr, "%s: null or invalid transDuo handle\n",
                __func__);
    }
    return ret;
} // transDuoBufferLengthGet

/*
 * Free all resources used by "transDuo".
 */
void transDuoDestroy(struct TransDuo *transDuo)
{
    if(transDuo && transDuo->magic == globalMagic)
    {
        dptr *check = 
            find_dptr(&(transDuo->manager->tdstore), matchesTransDuo, transDuo);
        if(check)
        {
            remove_dptr(&(transDuo->manager->tdstore), check);
        }
        else
        {
            fprintf(stderr, "%s: Got what looks to be a valid transDuo "
                    "but couldn't find it in the store\n", __func__);
        }
        transDuo->magic = 0;
        if(transDuo->buffer)
        {
            free(transDuo->buffer);
        }
        transformHandleDestroy(transDuo);
        free(transDuo);
    }
    else
    {
        fprintf(stderr, "%s: null or invalid transDuo handle\n",
                __func__);
    }
    return;
} // transDuoDestroy

/**************************
 * TransDuo buffer functions
 **************************/

/*
 * Add a buffer to the transDuo
 *
 * Returns zero on success.
 */
int transDuoAdd(
    struct TransDuo *transDuo,
    void const *buf,
    size_t buflen,
    Transform *transform)
{
    int ret;
    if(transDuo && transDuo->magic == globalMagic)
    {
        TransformDataHandle h = transform->dataHandleCreate(transform);
        if(h)
        {
            if((ret = transform->setRaw(h, buf, buflen)) == 0)
            {
                transformHandleDestroy(transDuo);
                if(transDuo->buffer)
                {
                    free(transDuo->buffer);
                    transDuo->buffer = 0;
                }
                transDuo->direction = TRANSFORM;
                transDuo->uort.t.x = transform;
                transDuo->uort.t.h = h;
            }
            else
            {
                fprintf(stderr, "%s: Failed to set the buffer data. "
                        "\"%s\" (%d)\n", __func__, strerror(ret), ret);
            }
        }
        else
        {
            ret = EINVAL; // don't really know what went wrong
            fprintf(stderr, 
                    "%s: Failed to create transform data handle\n",
                    __func__);
        }
    }
    else
    {
        ret = 0;
        fprintf(stderr, "%s: null or invalid transDuo handle\n",
                __func__);
    }
    return ret;
} // transDuoAdd

/*
 * Copy a transDuo from one transDuo to another.
 *
 * The transDuo may get modified upon copy. This occurs if, for example,
 * the transform is for multiple signatures
 *
 * Returns zero on success.
 */
int transDuoCopy(
        struct TransDuo *dst,
        struct TransDuo *src, 
        Transform *transform)
{
    int ret;
    int srctag = transDuoTagGet(src);
    if(dst && dst->magic == globalMagic && 
            srctag != -1 && (uint32_t)srctag == transform->tag)
    {
        void const *transformed;
        size_t transformedLength;
        if((ret = loadTransformed(src, &transformed,
                        &transformedLength)) == 0)
        {
            TransformDataHandle h = transform->dataHandleCreate(transform);
            if(h)
            {
                if((ret = transform->copy(
                                h, 
                                transformed,
                                transformedLength,
                                TRANSFORM_COPY_AND_AMEND)) == 0)
                {
                    transformHandleDestroy(dst);
                    if(dst->buffer)
                    {
                        free(dst->buffer);
                        dst->buffer = 0;
                    }
                    dst->direction = TRANSFORM;
                    dst->uort.t.x = transform;
                    dst->uort.t.h = h;
                }
                else
                {
                    fprintf(stderr, "%s: Failed to copy in the buffer data. "
                            "\"%s\" (%d)\n", __func__, strerror(ret), ret);
                }
            }
            else
            {
                ret = EINVAL; // don't really know what went wrong
                fprintf(stderr, "%s: Failed to create transform handle\n",
                        __func__);
            }
        }
        else
        {
            fprintf(stderr, "%s: Failed to get transformed data from "
                    "\"src\"\n", __func__);
        }
    }
    else
    {
        ret = EINVAL;
        if(srctag != -1 && (uint32_t)srctag != transform->tag)
        {
            fprintf(stderr, "%s: transform of src and the given "
                    "transform don't match (%d != %d)\n",
                    __func__, srctag, transform->tag);
        }
        else
        {
            fprintf(stderr, "%s: null or invalid transDuo handle\n",
                    __func__);
        }
    }
    return ret;
} // transDuoCopy

/*
 * Get the transDuo transform/untransform tag.
 *
 * Returns -1 on failure (we assume no tranform uses "-1" as its tag).
 */
int transDuoTagGet(struct TransDuo *transDuo)
{
    int ret;
    if(transDuo && transDuo->magic == globalMagic)
    {
        if(transDuo->direction == TRANSFORM)
        {
            if(transDuo->uort.t.x)
            {
                ret = transDuo->uort.t.x->tag;
            }
            else
            {
                ret = -1;
                fprintf(stderr, "%s: No transform set\n", __func__);
            }
        }
        else
        {
            if(transDuo->uort.u.x)
            {
                ret = transDuo->uort.u.x->tag;
            }
            else
            {
                ret = -1;
                fprintf(stderr, "%s: No untransform set\n", __func__);
            }
        }
    }
    else
    {
        ret = -1;
        fprintf(stderr, "%s: null or invalid transDuo handle\n",
                __func__);
    }
    return ret;
} // transDuoTagGet

/*
 * Returns a transform data handle for the transDuo. Given the tag 
 * and the handle, more information can be gleaned from the transDuo 
 * directly from the transformation code.
 *
 * For instance, if the transformation is for a set of signatures, there
 * should be a way to get the signers of the transDuo by calling
 * additional transformation proceedures that take the handle as a
 * parameter.
 *
 * Note that the transDuo holds either a transform or an untransform, not
 * both and the caller should call this or the similar
 * "transDuoUntransformDataHandleGet()" depending on what the transDuo holds. 
 * It is up to the caller to keep track of what the transDuo holds (it 
 * should be obvious given the context - outbound transDuos are 
 * transformed, inbound are untransformed).
 *
 * Returns 0 on failure.
 */
TransformDataHandle transDuoTransformDataHandleGet(struct TransDuo *transDuo)
{
    TransformDataHandle ret;
    if(transDuo && transDuo->magic == globalMagic)
    {
        if(transDuo->direction == TRANSFORM)
        {
            ret = transDuo->uort.t.h;
            if(ret == 0)
            {
                fprintf(stderr, "%s: Attempt to get a transfrom "
                        "data handle from a transDuo that has not been "
                        "loaded\n", __func__);
            }
        }
        else
        {
            ret = 0;
            fprintf(stderr, "%s: Attempt to get a transform data "
                    "handle from an untransform transDuo\n", __func__);
        }
    }
    else
    {
        ret = 0;
        fprintf(stderr, "%s: null or invalid transDuo handle\n",
                __func__);
    }
    return ret;
} // transDuoTransformDataHandleGet

/*
 * Returns an untransform data handle for the transDuo. Given the tag 
 * and the handle, more information can be gleaned from the transDuo 
 * directly from the untransformation code.
 *
 * For instance, if the untransformation is for a set of signatures, 
 * there should be a way to get the signers of the transDuo by calling
 * additional untransformation proceedures that take the handle as a
 * parameter.
 *
 * Note that the transDuo holds either a transform or an untransform, not
 * both and the caller should call this or the similar
 * "transDuoTransformDataHandleGet()" depending on what the transDuo holds. 
 * It is up to the caller to keep track of what the transDuo holds (it 
 * should be obvious given the context - outbound transDuos are 
 * transformed, inbound are untransformed).
 *
 * Returns 0 on failure.
 */
UntransformDataHandle transDuoUntransformDataHandleGet(struct TransDuo *transDuo)
{
    UntransformDataHandle ret;
    if(transDuo && transDuo->magic == globalMagic)
    {
        if(transDuo->direction == UNTRANSFORM)
        {
            ret = transDuo->uort.u.h;
            if(ret == 0)
            {
                fprintf(stderr, "%s: Attempt to get an untransfrom "
                        "data handle from a transDuo that has not been "
                        "loaded\n", __func__);
            }
        }
        else
        {
            ret = 0;
            fprintf(stderr, "%s: Attempt to get an untransform data "
                    "handle from a transform transDuo\n", __func__);
        }
    }
    else
    {
        ret = 0;
        fprintf(stderr, "%s: null or invalid transDuo handle\n",
                __func__);
    }
    return ret;
} // transDuoUntransformDataHandleGet

/*
 * Get the raw version of the transDuo.
 *
 * Returns null on failure. This can fail if a transDuo was copied
 * into this transDuo from another transDuo. In this case there is not a
 * raw version available to return.
 */
void const *transDuoRawGet(struct TransDuo *transDuo)
{
    void const *ret;
    if(transDuo && transDuo->magic == globalMagic)
    {
        if(transDuo->direction == TRANSFORM)
        {
            if(transDuo->uort.t.x)
            {
                if(transDuo->uort.t.h)
                {
                    ret = transDuo->uort.t.x->raw(transDuo->uort.t.h);
                    if(ret == 0)
                    {
                        fprintf(stderr, "%s: Failed to get raw data "
                                "from transform data handle\n", 
                                __func__);
                    }
                }
                else
                {
                    ret = 0;
                    fprintf(stderr, "%s: no transform data\n", __func__);
                }
            }
            else
            {
                ret = 0;
                fprintf(stderr, "%s: no transform set\n", __func__);
            }
        }
        else
        {
            if(transDuo->uort.u.x)
            {
                if(transDuo->uort.u.h)
                {
                    ret = transDuo->uort.u.x->raw(transDuo->uort.u.h);
                    if(ret == 0)
                    {
                        fprintf(stderr, "%s: Failed to get raw data "
                                "from untransform data handle\n", 
                                __func__);
                    }
                }
                else
                {
                    ret = 0;
                    fprintf(stderr, "%s: no untransform data\n", __func__);
                }
            }
            else
            {
                ret = 0;
                fprintf(stderr, "%s: no untransform set\n", __func__);
            }
        }
    }
    else
    {
        ret = 0;
        fprintf(stderr, "%s: null or invalid transDuo handle\n",
                __func__);
    }
    return ret;
} // transDuoRawGet

/*
 * Get the length of the data returned by "transDuoRawGet()"
 *
 * Returns zero on failure.
 */
size_t transDuoRawLengthGet(struct TransDuo *transDuo)
{
    size_t ret;
    if(transDuo && transDuo->magic == globalMagic)
    {
        if(transDuo->direction == TRANSFORM)
        {
            if(transDuo->uort.t.x)
            {
                if(transDuo->uort.t.h)
                {
                    ret = transDuo->uort.t.x->rawLength(transDuo->uort.t.h);
                }
                else
                {
                    ret = 0;
                    fprintf(stderr, "%s: no transform data\n", __func__);
                }
            }
            else
            {
                ret = 0;
                fprintf(stderr, "%s: no transform set\n", __func__);
            }
        }
        else
        {
            if(transDuo->uort.u.x)
            {
                if(transDuo->uort.u.h)
                {
                    ret = transDuo->uort.u.x->rawLength(transDuo->uort.u.h);
                }
                else
                {
                    ret = 0;
                    fprintf(stderr, "%s: no untransform data\n", __func__);
                }
            }
            else
            {
                ret = 0;
                fprintf(stderr, "%s: no untransform set\n", __func__);
            }
        }
    }
    else
    {
        ret = 0;
        fprintf(stderr, "%s: null or invalid transDuo handle\n",
                __func__);
    }
    return ret;
} // transDuoRawLengthGet

/*
 * Get the transformed version of the transDuo.
 *
 * Returns null on failure.
 */
void const *transDuoTransformedGet(struct TransDuo *transDuo)
{
    void const *ret;
    if(transDuo && transDuo->magic == globalMagic)
    {
        if(transDuo->direction == TRANSFORM)
        {
            if(transDuo->uort.t.x)
            {
                if(transDuo->uort.t.h)
                {
                    ret = transDuo->uort.t.x->transformed(
                            transDuo->uort.t.h);
                    if(ret == 0)
                    {
                        fprintf(stderr, "%s: Failed to get transformed "
                                "data from transform data handle\n", 
                                __func__);
                    }
                }
                else
                {
                    ret = 0;
                    fprintf(stderr, "%s: no transform data\n", 
                            __func__);
                }
            }
            else
            {
                ret = 0;
                fprintf(stderr, "%s: no transform set\n", __func__);
            }
        }
        else
        {
            if(transDuo->uort.u.x)
            {
                if(transDuo->uort.u.h)
                {
                    ret = transDuo->uort.u.x->transformed(
                            transDuo->uort.u.h);
                    if(ret == 0)
                    {
                        fprintf(stderr, "%s: Failed to get transformed "
                                "data from untransform data handle\n", 
                                __func__);
                    }
                }
                else
                {
                    ret = 0;
                    fprintf(stderr, "%s: no untransform data\n", 
                            __func__);
                }
            }
            else
            {
                ret = 0;
                fprintf(stderr, "%s: no untransform set\n", __func__);
            }
        }
    }
    else
    {
        ret = 0;
        fprintf(stderr, "%s: null or invalid transDuo handle\n",
                __func__);
    }
    return ret;
} // transDuoTransformedGet

/*
 * Get the length of the data returned by "transDuoTransformedGet()"
 *
 * Returns zero on failure.
 */
size_t transDuoTransformedLengthGet(struct TransDuo *transDuo)
{
    size_t ret;
    if(transDuo && transDuo->magic == globalMagic)
    {
        if(transDuo->direction == TRANSFORM)
        {
            if(transDuo->uort.t.x)
            {
                if(transDuo->uort.t.h)
                {
                    ret = transDuo->uort.t.x->transformedLength(
                            transDuo->uort.t.h);
                }
                else
                {
                    ret = 0;
                    fprintf(stderr, "%s: no transform data\n", 
                            __func__);
                }
            }
            else
            {
                ret = 0;
                fprintf(stderr, "%s: no transform set\n", __func__);
            }
        }
        else
        {
            if(transDuo->uort.u.x)
            {
                if(transDuo->uort.u.h)
                {
                    ret = transDuo->uort.u.x->transformedLength(
                            transDuo->uort.u.h);
                }
                else
                {
                    ret = 0;
                    fprintf(stderr, "%s: no untransform data\n", 
                            __func__);
                }
            }
            else
            {
                ret = 0;
                fprintf(stderr, "%s: no untransform set\n", __func__);
            }
        }
    }
    else
    {
        ret = 0;
        fprintf(stderr, "%s: null or invalid transDuo handle\n",
                __func__);
    }
    return ret;
} // transDuoTransformedLengthGet

