/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Affero General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Affero General Public License for more details.
 * 
 *     You should have received a copy of the GNU Affero General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef IDS_MESSAGE_TYPES_H
#define IDS_MESSAGE_TYPES_H

#include "idsCommunications.h"

/*
 * Defines canonical IdsCommunications message types.
 */


/* Values 0x0000 to 0xFFFF are reserved
 */

/* obsolete:
 * #define IDSCOMMUNICATIONS_MESSAGE_IDMEF					0x00010010
 */
#define IDSCOMMUNICATIONS_MESSAGE_IDMEF_ALERT				0x00010011
#define IDSCOMMUNICATIONS_MESSAGE_IDMEF_HEARTBEAT			0x00010012
#define IDSCOMMUNICATIONS_MESSAGE_IDMEF_ALERT_CONSOLIDATED		0x00010014
#define IDSCOMMUNICATIONS_MESSAGE_IDMEF_ALERT_CONSOLIDATED_TOROOTGROUP  0x00010015
#define IDSCOMMUNICATIONS_MESSAGE_IDMEF_ALERT_ACK			0x00010016
#define IDSCOMMUNICATIONS_MESSAGE_IDMEF_ALERT_VERIFIED		0x00010017

#define IDSCOMMUNICATIONS_MESSAGE_IDMEF_ALERT_CONSOLIDATED_SIGNED	0x00010024
#define IDSCOMMUNICATIONS_MESSAGE_IDMEF_ALERT_CONSOLIDATED_TOROOTGROUP_SIGNED 0x00010025
#define IDSCOMMUNICATIONS_MESSAGE_IDMEF_ALERT_ACK_SIGNED		0x00010026

/*message used by response generator
  */
#define IDSCOMMUNICATIONS_MESSAGE_RG_BLOCK       0x00010013

/* messages used by demodetector and friends 
 */
#define IDSCOMMUNICATIONS_MESSAGE_DEMO_REPORT				0x00010042
#define IDSCOMMUNICATIONS_MESSAGE_DEMO_DIRECTIVE			0x00010043
#define IDSCOMMUNICATIONS_MESSAGE_DEMO_MESSAGE				0x00010044
#define IDSCOMMUNICATIONS_MESSAGE_DEMO_SIGNED_REPORT		0x00010045
#define IDSCOMMUNICATIONS_MESSAGE_DEMO_SIGNED_DIRECTIVE		0x00010046

/* messages used by the inconsistent routing detector
 */
#define IDSCOMMUNICATIONS_MESSAGE_INCONSISTANCY_LINKSTATE		0x00010100

/* messages used by the link history querier
 */
#define IDSCOMMUNICATIONS_MESSAGE_LINKHISTORYQUERY			0x00010101
#define IDSCOMMUNICATIONS_MESSAGE_LINKHISTORYREPLY			0x00010102

/* Messages used to talk to the watcher
 */
#define IDSCOMMUNICATIONS_MESSAGE_WATCHER_LABEL				0x00010200
#define IDSCOMMUNICATIONS_MESSAGE_WATCHER_LABEL_REMOVE			0x00010201
#define IDSCOMMUNICATIONS_MESSAGE_WATCHER_COLOR				0x00010202
#define IDSCOMMUNICATIONS_MESSAGE_WATCHER_EDGE				0x00010203
#define IDSCOMMUNICATIONS_MESSAGE_WATCHER_EDGE_REMOVE			0x00010204
#define IDSCOMMUNICATIONS_MESSAGE_WATCHER_GPS				0x00010205
#define IDSCOMMUNICATIONS_MESSAGE_WATCHER_GRAPH				0x00010206
#define IDSCOMMUNICATIONS_MESSAGE_WATCHER_GRAPH_EDGE			0x00010207
#define IDSCOMMUNICATIONS_MESSAGE_WATCHER_FLOATINGLABEL			0x00010208
#define IDSCOMMUNICATIONS_MESSAGE_WATCHER_FLOATINGLABEL_REMOVE		0x00010209
#define IDSCOMMUNICATIONS_MESSAGE_WATCHER_PROPERTY		0x0001020A

 /* messages used by the wormhole detector 
  */
#define IDSCOMMUNICATIONS_MESSAGE_WORMHOLE_DATA				0x00010300

/* messages used by the Lipad detector
*/
#define IDSCOMMUNICATIONS_MESSAGE_LIPAD_REPORT				0x00010400

typedef struct CommunicationsMessageType
{
	MessageType type;
	const void *dictionary;
	int dictionarylen;
} CommunicationsMessageType;

#endif
