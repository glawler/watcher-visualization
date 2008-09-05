/*
 * transDuo.h - manage a buffer that has a raw and a transformed state.
 *
 * Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 */
#ifndef TRANSDUO_H_FILE
#define TRANSDUO_H_FILE

#include <unistd.h> /* for size_t */

#include "transform.h"
#include "untransform.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************************/
/*
 * The TransDuo subsystem
 */

/*
 * Needed to create TransDuo handles and to register untransforms.
 */
struct TransDuoManager;

/*
 * Perform any transDuo initialization processing.
 */
void transDuoInit(void);

/*
 * Will clean up any transDuo global data
 */
void transDuoFini(void);

/*
 * Create a handle to a TransDuo manager
 */
struct TransDuoManager *transDuoManagerCreate(void);

/*
 * Destroy a TransDuo manager handle created in
 * "transDuoManagerCreate()"
 */
void transDuoManagerDestroy(struct TransDuoManager *h);

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
void transDuoUntransformRegister(
        struct TransDuoManager *m, 
        Untransform *u);

/**********************************************************************/
/*
 * TransDuo management
 *
 * To work with a transDuo, you first get a transDuo handle. A transDuo
 * handle can be created as an empty transDuo or it can be created from a
 * previously marshaled transDuo, in which case it will be ready to use.
 *
 * Given a transDuo handle, you can add a buffer or retrieve a buffer.
 * When retrieving a buffer, the buffer can be raw or transformed.
 *
 * Also, given a transDuo handle, you can marshall the buffer into a
 * single contiguous byte array.
 */
/**********************************************************************/

struct TransDuo;

/***********************
 * TransDuo functions
 ***********************/

/*
 * Create an empty transDuo handle.
 *
 * Returns 0 on failure to allocate.
 */
struct TransDuo *transDuoCreate(struct TransDuoManager *m);

/*
 * Create a transDuo from a buffer. The buffer must be formatted as 
 * the result from "transDuoBufferGet()"
 */
struct TransDuo *transDuoFromBuffer(
        struct TransDuoManager *m,
        const void *buffer, 
        size_t bufferLength);

/*
 * Get the buffer that is the raw buffer transformed and
 * marshaled into a byte array. That is formatted so
 * "transDuoFromBuffer()" can parse it.
 *
 * The pointer returned is valid until until a call to transDuoAdd(), the
 * TransDuo is the destination of a transDuoCopy(), or the TransDuo is destroyed
 * by a "transDuoDestroy()" call.
 */
void const *transDuoBufferGet(struct TransDuo *transDuo);

/*
 * Allocate and return a copy of the buffer returned in
 * "transDuoBufferGet()".
 *
 * Free this with "free()".
 */
void *transDuoBufferDup(struct TransDuo *transDuo);

/*
 * Get the length of the buffer returned by "transDuoGetBuffer()"
 *
 * The length returned is valid until until a call to transDuoAdd(), the
 * TransDuo is the destination of a transDuoCopy(), or the TransDuo is destroyed
 * by a "transDuoDestroy()" call.
 */
size_t transDuoBufferLengthGet(struct TransDuo *transDuo);

/*
 * Free all resources used by "transDuo".
 */
void transDuoDestroy(struct TransDuo *transDuo);

/***********************
 * TransDuo buffer functions
 ***********************/

/*
 * Add a buffer to the transDuo
 *
 * Returns zero on success.
 */
int transDuoAdd(
    struct TransDuo *transDuo,
    void const *buf,
    size_t buflen,
    Transform *transform);

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
        Transform *transform);

/*
 * Get the transDuo transform/untransform tag.
 *
 * Returns -1 on failure (we assume no tranform uses "-1" as its tag).
 */
int transDuoTagGet(struct TransDuo *transDuo);

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
 * Note that the TransDuo holds either a Transform or an Untransform, not
 * both and the caller should call this or the similar
 * "transDuoUntransformDataHandleGet()" depending on what the TransDuo holds. 
 * It is up to the caller to keep track of what the TransDuo holds (it 
 * should be obvious given the context - outbound TransDuos are 
 * transformed, inbound are untransformed).
 *
 * The handle returned is valid until until a call to transDuoAdd(), the
 * TransDuo is the destination of a transDuoCopy(), or the TransDuo is destroyed
 * by a "transDuoDestroy()" call.
 *
 * Returns 0 on failure.
 */
TransformDataHandle transDuoTransformDataHandleGet(struct TransDuo *transDuo);

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
 * The handle returned is valid until until a call to transDuoAdd(), the
 * TransDuo is the destination of a transDuoCopy(), or the TransDuo is destroyed
 * by a "transDuoDestroy()" call.
 *
 * Returns 0 on failure.
 */
UntransformDataHandle transDuoUntransformDataHandleGet(struct TransDuo *transDuo);

/*
 * Get the raw version of the transDuo.
 *
 * The pointer returned is valid until until a call to transDuoAdd(), the
 * TransDuo is the destination of a transDuoCopy(), or the TransDuo is destroyed
 * by a "transDuoDestroy()" call.
 *
 * Returns null on failure. This can fail if a transDuo was copied
 * into this transDuo from another transDuo. In this case there is not a
 * raw version available to return.
 */
void const *transDuoRawGet(struct TransDuo *transDuo);

/*
 * Get the length of the data returned by "transDuoRawGet()"
 *
 * The length returned is valid until until a call to transDuoAdd(), the
 * TransDuo is the destination of a transDuoCopy(), or the TransDuo is destroyed
 * by a "transDuoDestroy()" call.
 *
 * Returns zero on failure.
 */
size_t transDuoRawLengthGet(struct TransDuo *transDuo);

/*
 * Get the transformed version of the transDuo.
 *
 * The pointer returned is valid until until a call to transDuoAdd(), the
 * TransDuo is the destination of a transDuoCopy(), or the TransDuo is destroyed
 * by a "transDuoDestroy()" call.
 *
 * Returns null on failure.
 */
void const *transDuoTransformedGet(struct TransDuo *transDuo);

/*
 * Get the length of the data returned by "transDuoTransformedGet()"
 *
 * The length returned is valid until until a call to transDuoAdd(), the
 * TransDuo is the destination of a transDuoCopy(), or the TransDuo is destroyed
 * by a "transDuoDestroy()" call.
 *
 * Returns zero on failure.
 */
size_t transDuoTransformedLengthGet(struct TransDuo *transDuo);


#ifdef __cplusplus
}
#endif

#endif /* TRANSDUO_H_FILE */
