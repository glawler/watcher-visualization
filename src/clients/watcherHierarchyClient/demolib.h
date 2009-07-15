/** 
 * @file demolib.h
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
#ifndef DEMOLIB_H
#define DEMOLIB_H

#include "idsCommunications.h"

/*  Copyright (C) 2004  Networks Associates Technology, Inc.
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#define PRINTADDR(a) ((a)>>24)&0xFF,((a)>>16)&0xFF,((a)>>8)&0xFF,(a)&0xFF

void detectorMessageStatus(const struct MessageInfo *mi, void *messageStatusHandlerData,MessageStatus status);
void detectorPositionUpdate(void *data, IDSPositionType position, IDSPositionStatus status);
CommunicationsStatePtr detectorCommsInit(ManetAddr us, 
                                         const char *readlog, 
                                         const char *writelog, 
                                         const char *name);

#ifdef __cplusplus
};
#endif

#endif
