/*
 * idmefPrint.h
 */
#ifndef IDMEFPRINT_H_FILE
#define IDMEFPRINT_H_FILE

#include <stdio.h> /* for FILE */
#include <libxml/tree.h> /* for xmlDocPtr */

#ifdef __cplusplus
# define EXTERNC extern "C"
#else
# define EXTERNC
#endif

EXTERNC void idmefPrint(FILE *fil, xmlDocPtr messagedoc);

#endif /* IDMEFPRINT_H_FILE */
