/*
 * transformNull.h - perform a null transformation 
 */
#ifndef TRANSFORM_NULL_H_FILE
#define TRANSFORM_NULL_H_FILE

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#include "transform.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Create a null Transform
 */
Transform *transformNullCreate(void);

extern const int transformNullTag;

#ifdef __cplusplus
}

#endif
#endif /* TRANSFORM_NULL_H_FILE */
