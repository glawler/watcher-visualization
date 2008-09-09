/*
 * untransformSign.h - perform a signature untransformation 
 */
#ifndef UNTRANSFORM_SIGN_H_FILE
#define UNTRANSFORM_SIGN_H_FILE

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#include "untransform.h"
#include "idsCommunications.h" /* for ManetAddr */

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    UntransformSignVerified,
    UntransformSignVerificationFailure,
    UntransformSignReplay,
    UntransformSignNoPeer,
    UntransformSignInternalError
} UntransformSignResult;

#define UNTRANSFORM_SIGN_RESULT_TEXT(res) \
    ((res) == UntransformSignVerified ? "Verified" : \
     (res) == UntransformSignVerificationFailure ? "Verification Failure" : \
     (res) == UntransformSignReplay ? "Replay" : \
     (res) == UntransformSignNoPeer ? "No Peer" : \
     (res) == UntransformSignInternalError ? "Internal Error" : "Unknown")

/*
 * Reads the file and uses every line found with the format:
 *
 * d.d.d.d <N>|<e>|<chk1><blah>
 *
 *   d.d.d.d - dotted decimal IPv4 address to use as an identifier.
 *   N       - modulus
 *   e       - public exponent (unused)
 *   chk1    - sha256-64 of N and e
 *   blah    - can be anything that is not chk1
 *
 * The formats of "<N>", "<e>", and "<chk1>" are as understood by
 * "string_to_fp_int()".
 *
 * The '#' character starts a comment which runs to the end of the line.
 */
Untransform *untransformSignCreate(char const *fname);

/*
 * Return the signer.
 *
 * Returns 0 on failure. Failure occurs when the signer cannot be parsed
 * - this can happen on buffers that are too short.
 */
ManetAddr untransformSignSigner(UntransformDataHandle handle);

/*
 * Return the result of the untranform.
 */
UntransformSignResult 
untransformSignResult(UntransformDataHandle handle);

#ifdef __cplusplus
}

#endif
#endif /* UNTRANSFORM_SIGN_H_FILE */
