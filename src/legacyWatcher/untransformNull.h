/*
 * untransformNull.h - perform a null untransformation 
 */
#ifndef UNTRANSFORM_NULL_H_FILE
#define UNTRANSFORM_NULL_H_FILE

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#include "untransform.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Create a null Untransform
 */
Untransform *untransformNullCreate(void);

extern const int untransformNullTag;

#ifdef __cplusplus
}

#endif
#endif /* UNTRANSFORM_NULL_H_FILE */
