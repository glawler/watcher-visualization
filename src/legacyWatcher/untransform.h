/*
 * untransform.h - work with transformed byte arrays.
 *
 * Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 */
#ifndef UNTRANSFORM_H_FILE
#define UNTRANSFORM_H_FILE

#include <stdint.h> /* for uint32_t */
#include <unistd.h> /* for size_t */

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/*
 * "UntransformDataHandle" is used to keep track of a raw/transformed data
 * pair along with any other associated information.
 *
 * The "UntransformDataHandleData" structure probably doesn't exist. The
 * different untransform types cast to and from the handle to their own
 * private formats.
 */
typedef struct UntransformDataHandleData *UntransformDataHandle;

/*
 * Generic way to undo what Transform does. Mostly a pile of
 * function pointers to by used by the payload processing code.
 */
typedef struct Untransform
{
    /*
     * Used by the untransform.
     */
    void *untransformData;
    /*
     * Used to find the untransform. Transforms that can, such as
     * signatures and MACs, should bind the tag value to the transformed
     * value.
     *
     * Must match the value in "Transform.tag"
     */
    uint32_t tag;
    /*
     * Create a handle to pass into the other functions.
     */
    UntransformDataHandle (*dataHandleCreate)(struct Untransform *);
    /*
     * Set transformed data to untransform.
     *
     * Returns zero on success.
     */
    int (*setTransformed)(
            UntransformDataHandle handle, 
            void const *transformed, 
            size_t transformedLength);
    /*
     * Return the raw data from untransforming the data passed into
     * "setTransformed()"
     * 
     * Returns null on failure.
     */
    void const * (*raw)(const UntransformDataHandle handle);
    /*
     * Return the length of the buffer returned by "raw()" 
     *
     * Retuns zero on failure.
     */
    size_t (*rawLength)(const UntransformDataHandle handle);
    /*
     * Return the transformed data passed into "setTransformed()"
     *
     * Retuns null on failure.
     */
    void const * (*transformed)(const UntransformDataHandle handle);
    /*
     * Return the length of the buffer returned by "transformed()" 
     *
     * Retuns zero on failure.
     */
    size_t (*transformedLength)(const UntransformDataHandle handle);
    /*
     * Cleans up resources owned by "handle". Note that the buffer
     * returned by "raw()" or "transformed()" is also cleaned up.
     */
    void (*dataHandleDestroy)(UntransformDataHandle handle);
    /*
     * Cleans up resources owned by "untransform". Only works
     * on an Untransform like "u->untransformDestroy(u)" - constructs
     * like "u1->untransformDestroy(u2)" will likely fail. Do not use
     * the Untransform after passing it into a untransformDestroy()
     * function.
     */
    void (*untransformDestroy)(struct Untransform *untransform);
} Untransform;

#endif /* UNTRANSFORM_H_FILE */
