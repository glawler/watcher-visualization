#ifndef FLOATING_LABEL_H
#define FLOATING_LABEL_H

/* $Id: floatinglabel.h,v 1.2 2007/04/25 14:20:05 dkindred Exp $
 */

/* this contains the declaration of a FloatingLabel and NodeDisplayStatus
 */
#include "idsCommunications.h"

#ifdef __cplusplus
extern "C" {
#endif

void floatingLabelAdd(FloatingLabel **list, FloatingLabel *lab,destime curtime);
void floatingLabelRemove(FloatingLabel **g, int bitmap, FloatingLabel *nw);
void floatingLabelRemoveFamily(FloatingLabel **g,int family);
void floatingLabelNuke(FloatingLabel **g);
void floatingLabelDraw(FloatingLabel **list, NodeDisplayType dispType, NodeDisplayStatus *dispStat, destime curtime);

#ifdef __cplusplus
}
#endif

#endif
