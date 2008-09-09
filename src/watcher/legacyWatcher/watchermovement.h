/* $Id: watchermovement.h,v 1.6 2007/04/25 14:20:05 dkindred Exp $
 *
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 */

#ifndef WATCHERMOVEMENT_H
#define WATCHERMOVEMENT_H

#include "des.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Called by watcher to initialize the Motion library
 *
 * returns a pointer to its state info, which will be passed back
 * to any other watcherMotion calls.
 */
void *watcherMovementInit(manet *m);

/* if the watcherMotion implementation needs to listen on a
 * file descriptor, it may return it with this call (which the
 * watcher will call regularly in its select loop.)  Return -1
 * to indicate there is no FD.
 */
int watcherMovementFD(void *motionData);

/* When the FD returned by watcherMotionFD is readable, the 
 * watcher will call this function.  (If there is no FD,
 * this function WILL NOT be called.)
 *
 * return non-zero if there is actually new data.
 */
int watcherMovementRead(void *motionData, manet *m);

/* If there is no FD, this function will be called regularly.
 *
 * return non-zero if there is actually new data.
 */
int watcherMovementUpdate(void *motionData, manet *m);

#ifdef __cplusplus
}
#endif

#endif
