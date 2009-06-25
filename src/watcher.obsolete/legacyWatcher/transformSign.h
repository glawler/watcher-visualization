/*
 * transformSign.h - perform a signature transformation 
 */
#ifndef TRANSFORM_SIGN_H_FILE
#define TRANSFORM_SIGN_H_FILE

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#include "transform.h"
#include "idsCommunications.h" /* for ManetAddr */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Reads the file and uses the first line found with the format:
 *
 * d.d.d.d <N>|<e>|<chk1>|<d>|<p>|<q>|<qP>|<dP>|<dQ>|<chk2><blah>
 *
 *   d.d.d.d - dotted decimal IPv4 address to use as an identifier.
 *   N       - modulus
 *   e       - public exponent (unused)
 *   chk1    - sha256-64 of N and e
 *   d       - private exponent
 *   p       - p factor of N
 *   q       - q factor of N
 *   qP      - 1/q mod p CRT param
 *   dP      - d mod (p - 1) CRT param
 *   dQ      - d mod (q - 1) CRT param
 *   chk2    - sha256-64 of N, d, p, q, dP, qP, dP, dQ
 *   blah    - can be anything that is not chk2
 *
 * See the formats of "N" through "chk2" are as understood by
 * "string_to_fp_int()".
 *
 * The '#' character starts a comment which runs to the end of the line.
 */
Transform *transformSignCreate(char const *fname);

#ifdef __cplusplus
}
#endif

#endif /* TRANSFORM_SIGN_H_FILE */
