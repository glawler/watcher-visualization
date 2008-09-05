#ifndef INTERIM2_H
#define INTERIM2_H

#include "des.h"
#include "hello.h"

/* Note that the statevector message types are in this space (0..2) --
 * see des.h.
 */
#define PACKET_INTERIM2_HELLO           (PACKET_INTERIM2|5)
#define PACKET_INTERIM2_ROOT            (PACKET_INTERIM2|6)

#ifdef __cplusplus
extern "C" {
#endif

void interim2PayloadCallbackSet(manetNode *us, helloHello *helloCallback);

void interim2PayloadSet(manetNode *us, unsigned char *payload, int payloadLen);

#ifdef __cplusplus
}
#endif

#endif /* !INTERIM2_H */
