/*
 * transform.h - work with byte arrays.
 *
 * Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 */
#ifndef TRANSFORM_H_FILE
#define TRANSFORM_H_FILE

#include <stdint.h> /* for uint32_t */
#include <unistd.h> /* for size_t */

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/**********************************************************************/
/*
 * Transformations:
 *
 * A byte array can be transformed in different ways.
 *
 * A typical transformation consists of cryptographic signing.
 *
 * Transformed byte arrays may be copied keeping their initial 
 * transformation (such as their initial signature) or they may be 
 * amended (like adding an additional signature) or they may be 
 * stripped of their transformation.
 */
/**********************************************************************/

/*
 * When copying byte arrays, there may need to be a straight copy or there
 * may need to be a copy that performs a transformations.
 */
typedef enum TransformCopyType
{
    TRANSFORM_COPY_STRAIGHT,
    TRANSFORM_COPY_AND_AMEND
} TransformCopyType;

/*
 * "TransformDataHandle" is used to keep track of a raw/transformed data
 * pair along with any other associated information.
 *
 * The "TransformDataHandleData" structure probably doesn't exist. The
 * different transform types cast to and from the handle to their own
 * private formats.
 */
typedef struct TransformDataHandleData *TransformDataHandle;

/*
 * Generic transform definition. Mostly a pile of function pointers to
 * by used by the payload processing code.
 */
typedef struct Transform
{
    /*
     * Used by the transform.
     */
    void *transformData;
    /*
     * Used to find the untransform. Transforms that can, such as
     * signatures and MACs, should bind the tag value to the transformed
     * value.
     *
     * Must match the value in "Untransform.tag"
     */
    uint32_t tag;
    /*
     * Create a handle to pass into the other functions.
     */
    TransformDataHandle (*dataHandleCreate)(struct Transform *);
    /*
     * Set raw data to transform.
     *
     * This invalidates any data passed into "copy()"
     *
     * Returns zero on success.
     */
    int (*setRaw)(
            TransformDataHandle handle, 
            void const *raw, 
            size_t rawLength);
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
    int (*copy)(
            TransformDataHandle handle, 
            void const *transformed, 
            size_t transformedLength, 
            TransformCopyType copyType);
    /*
     * Return the raw data from "setRaw()".
     * 
     * Returns null on failure.
     */
    void const * (*raw)(const TransformDataHandle handle);
    /*
     * Return the length of the buffer returned by "raw()" 
     *
     * Retuns zero on failure.
     */
    size_t (*rawLength)(const TransformDataHandle handle);
    /*
     * Return the transformed version of the data passed into "setRaw()"
     * or "copy()".
     *
     * Retuns null on failure.
     */
    void const * (*transformed)(const TransformDataHandle handle);
    /*
     * Return the length of the buffer returned by "transformed()" 
     *
     * Retuns zero on failure.
     */
    size_t (*transformedLength)(const TransformDataHandle handle);
    /*
     * Cleans up resources owned by "handle". Note that the buffer
     * returned by "transformed()" is also cleaned up.
     */
    void (*dataHandleDestroy)(TransformDataHandle handle);
    /*
     * Cleans up resources owned by "transform". Only works
     * on an Transform like "u->transformDestroy(u)" - constructs
     * like "u1->transformDestroy(u2)" will likely fail. Do not use
     * the Transform after passing it into a destroyTransform()
     * function.
     */
    void (*transformDestroy)(struct Transform *transform);
} Transform;

#endif /* TRANSFORM_H_FILE */
