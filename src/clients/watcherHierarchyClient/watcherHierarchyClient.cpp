#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <string.h>

#include "idsCommunications.h"
#include "demolib.h"

#include "legacyWatcherMessageUnmarshal.h"
#include "watcherGPS.h"
#include "labelMessage.h"
#include "edgeMessage.h"
#include "gpsMessage.h"
#include "colorMessage.h"
#include "client.h"
#include "logger.h"

using namespace std;
using namespace watcher;
using namespace watcher::event;
using namespace boost;
using namespace boost::asio;

/*  Copyright (C) 2004  Networks Associates Technology, Inc.
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 *
 * This is an example aggregator, to show how to use the Infrastructure API.
 * It listens for messages from the demodetector, and sends aggregated messages
 * every 2 seconds.
 *
 */

static const char *rcsid __attribute__ ((unused)) = "$Id: demoaggregator.c,v 1.24 2007/08/20 02:54:29 dkindred Exp $";

#define PRINTADDR(a) ((a)>>24)&0xFF,((a)>>16)&0xFF,((a)>>8)&0xFF,(a)&0xFF

typedef struct detector
{
    CommunicationsStatePtr cs;
    ClientPtr client;
} detector;

/* This is called by the API when this node's position in the hierarchy changes
 * It is defined using the API function idsPositionRegister().
 */
static void myDetectorPositionUpdate(void *, IDSPositionType position, IDSPositionStatus status)
{
    switch(position)
    {
        case COORDINATOR_ROOT: 
            LOG_DEBUG("Position change: root " << (status==IDSPOSITION_ACTIVE?"active":"inactive")); 
                break;
        case COORDINATOR_REGIONAL: 
            LOG_DEBUG("Position change: regional " << (status==IDSPOSITION_ACTIVE?"active":"inactive")); 
                break;
        case COORDINATOR_NEIGHBORHOOD:
            LOG_DEBUG("Position change: regional " << (status==IDSPOSITION_ACTIVE?"active":"inactive")); 
            break;
        case COORDINATOR_ROOTGROUP:
            LOG_DEBUG("Position change: regional " << (status==IDSPOSITION_ACTIVE?"active":"inactive")); 
            break;
    }
}

void sendLabel(void *messageHandlerData, const struct MessageInfo *mi, bool addLabel) 
{
    TRACE_ENTER();

    detector *st=(detector*)messageHandlerData;
    unsigned char *payload = static_cast<unsigned char *>(messageInfoRawPayloadGet(mi)); 
    size_t payloadLen = messageInfoRawPayloadLenGet(mi); 

    LOG_DEBUG("Received hierarchy label message of size " << payloadLen << ", unmarshalling it."); 

    // Annoying to have to preallocate the string's buffer.
    char textBuf[260];
    NodeLabel lab;
    lab.text=textBuf;

    communicationsWatcherLabelUnmarshal(payload, &lab);

    LabelMessagePtr lm(new LabelMessage);

    lm->label=lab.text;
    lm->fontSize=12;        // not in NodeLabel
    lm->fromNodeID=ip::address_v4(htonl(lab.node));     
    lm->foreground=Color(lab.fgcolor[0], lab.fgcolor[1], lab.fgcolor[2], lab.fgcolor[3]);
    lm->background=Color(lab.bgcolor[0], lab.bgcolor[1], lab.bgcolor[2], lab.bgcolor[3]);
    lm->expiration=lab.expiration;
    lm->addLabel=addLabel;

    st->client->sendMessage(lm); 
}

void sendLabelAdd(void *messageHandlerData, const struct MessageInfo *mi) 
{
    TRACE_ENTER();
    sendLabel(messageHandlerData, mi, true);
    TRACE_EXIT();
}

void sendLabelRemove(void *messageHandlerData, const struct MessageInfo *mi) 
{
    TRACE_ENTER();
    sendLabel(messageHandlerData, mi, false);
    TRACE_ENTER();
}

GUILayer legacyFamilyValue2GUILayer(unsigned int family)
{
    TRACE_ENTER();
    switch (family)
    {
        case COMMUNICATIONS_LABEL_FAMILY_UNDEFINED: return UNDEFINED_LAYER;
        case COMMUNICATIONS_LABEL_FAMILY_PHYSICAL: return PHYSICAL_LAYER;
        case COMMUNICATIONS_LABEL_FAMILY_HIERARCHY: return HIERARCHY_LAYER;
        case COMMUNICATIONS_LABEL_FAMILY_BANDWIDTH: return BANDWIDTH_LAYER;
        case COMMUNICATIONS_LABEL_FAMILY_ROUTING: return ROUTING_LAYER;
        case COMMUNICATIONS_LABEL_FAMILY_ROUTING_ONEHOP: return ONE_HOP_ROUTING_LAYER;
        case COMMUNICATIONS_LABEL_FAMILY_ANTENNARADIUS: return ANTENNARADIUS_LAYER;
        case COMMUNICATIONS_LABEL_FAMILY_SANITYCHECK: return SANITY_CHECK_LAYER;
        case COMMUNICATIONS_LABEL_FAMILY_ANOMPATHS: return ANOMPATHS_LAYER;
        case COMMUNICATIONS_LABEL_FAMILY_CORRELATION: return CORROLATION_LAYER;
        case COMMUNICATIONS_LABEL_FAMILY_ALERT: return ALERT_LAYER;
        case COMMUNICATIONS_LABEL_FAMILY_CORRELATION_3HOP: return CORROLATION_3HOP_LAYER;
        case COMMUNICATIONS_LABEL_FAMILY_ROUTING2: return ROUTING2_LAYER;
        case COMMUNICATIONS_LABEL_FAMILY_ROUTING2_ONEHOP: return ROUTING2_ONE_HOP_LAYER;
        case COMMUNICATIONS_LABEL_FAMILY_FLOATINGGRAPH: return FLOATING_GRAPH_LAYER;
        case COMMUNICATIONS_LABEL_FAMILY_NORMPATHS: return NORMAL_PATHS_LAYER; 
        default: return UNDEFINED_LAYER;
    }

    TRACE_EXIT(); 
}

void sendEdge(void *messageHandlerData, const struct MessageInfo *mi, bool addEdge)
{
    TRACE_ENTER();

    detector *st=(detector*)messageHandlerData;
    unsigned char *payload = static_cast<unsigned char *>(messageInfoRawPayloadGet(mi)); 
    size_t payloadLen = messageInfoRawPayloadLenGet(mi); 

    LOG_DEBUG("Received hierarchy edge message of size " << payloadLen << ", unmarshalling it."); 

    NodeEdge *ne=NULL;
    communicationsWatcherEdgeUnmarshal(payload, ne); // ne is malloc'd inside, so free it after.

    EdgeMessagePtr em(new EdgeMessage);

    // Add the basics.
    em->node1=ip::address_v4(htonl(ne->head));
    em->node2=ip::address_v4(htonl(ne->tail));
    em->edgeColor=Color(ne->color[0], ne->color[1], ne->color[2], ne->color[3]); 
    em->expiration=ne->expiration;
    em->layer=legacyFamilyValue2GUILayer(ne->family); 
    em->addEdge=addEdge;

    // Add any labels if we have them.
    NodeLabel *labels[] = { &ne->labelHead, &ne->labelMiddle, &ne->labelHead }; 
    for (unsigned int i = 0; i < sizeof(labels)/sizeof(labels[0]); i++)
        if (labels[i]->text) // Is there a better way to test for existence?
        {
            LabelMessagePtr mlp(new LabelMessage);
            mlp->label=labels[i]->text;
            mlp->fontSize=12;
            mlp->foreground=Color(labels[i]->fgcolor[0], labels[i]->fgcolor[1], labels[i]->fgcolor[2], labels[i]->fgcolor[3]);
            mlp->background=Color(labels[i]->bgcolor[0], labels[i]->bgcolor[1], labels[i]->bgcolor[2], labels[i]->bgcolor[3]);
            mlp->expiration=labels[i]->expiration;
            mlp->addLabel=addEdge;

            // cheating a little
            if (i==0)  // head
            {
                mlp->fromNodeID=ip::address_v4(htonl(ne->head));
                em->setNode1Label(mlp);
            }
            else if (i==1)  // middle
            {
                em->setMiddleLabel(mlp);
            }
            else if (i==2)  // tail
            {
                mlp->fromNodeID=ip::address_v4(htonl(ne->tail));
                em->setNode2Label(mlp);
            }
            else break; // Shouldn't happen
        }


    st->client->sendMessage(em);

    free(ne); 
}

void sendEdgeRemove(void *messageHandlerData, const struct MessageInfo * messageInfo)  
{
    TRACE_ENTER();
    sendEdge(messageHandlerData, messageInfo, false);
    TRACE_EXIT();
}
void sendEdgeAdd(void *messageHandlerData, const struct MessageInfo * messageInfo) 
{
    TRACE_ENTER();
    sendEdge(messageHandlerData, messageInfo, true);
    TRACE_EXIT();
}

void sendGPS(void *messageHandlerData, const struct MessageInfo *mi) 
{
    TRACE_ENTER();

    detector *st=(detector*)messageHandlerData;
    unsigned char *payload = static_cast<unsigned char *>(messageInfoRawPayloadGet(mi)); 
    size_t payloadLen = messageInfoRawPayloadLenGet(mi); 

    LOG_DEBUG("Received GPS message of size " << payloadLen << ", unmarshalling it."); 

    WatcherGPS wGPS;
    watcherGPSUnmarshal(payload, payloadLen, &wGPS);

    GPSMessagePtr gpsMessage(new GPSMessage);
    gpsMessage->x=wGPS.lon;
    gpsMessage->y=wGPS.lat;
    gpsMessage->z=wGPS.alt;

    st->client->sendMessage(gpsMessage);

    TRACE_EXIT();
}

// GTL - I dont' really know what sending a color message is suppoed to do.
void sendWatcherColor(void *messageHandlerData, const struct MessageInfo *mi) 
{
    TRACE_ENTER();

    detector *st=(detector*)messageHandlerData;
    unsigned char *payload = static_cast<unsigned char *>(messageInfoRawPayloadGet(mi)); 
    size_t payloadLen = messageInfoRawPayloadLenGet(mi); 

    LOG_DEBUG("Received Color message of size " << payloadLen << ", unmarshalling it."); 

    unsigned char color[4];
    uint32_t nodeAddr;
    watcherColorUnMarshal(payload, &nodeAddr, color);

    ColorMessagePtr cm(new ColorMessage); 
    cm->fromNodeID=ip::address_v4(nodeAddr);
    cm->color=Color(color[0], color[1], color[2], color[3]);

    st->client->sendMessage(cm);

    TRACE_EXIT();
}

void sendGraph(void *, const struct MessageInfo *) 
{
    TRACE_ENTER();
    LOG_INFO("Ignoring 3d graph message"); 
    TRACE_EXIT();
}
void sendGraphEdge(void *, const struct MessageInfo *) 
{
    TRACE_ENTER();
    LOG_INFO("Ignoring 3d graph edge message"); 
    TRACE_EXIT();
}
void sendFloatinglabel(void *messageHandlerData, const struct MessageInfo *mi, bool addLabel)
{
    TRACE_ENTER();

    detector *st=(detector*)messageHandlerData;
    unsigned char *payload = static_cast<unsigned char *>(messageInfoRawPayloadGet(mi)); 
    size_t payloadLen = messageInfoRawPayloadLenGet(mi); 

    FloatingLabel lab;
    char string[260];
    // unsigned char *pos;
    lab.text = string;

    LOG_DEBUG("Received hierarchy floating label message of size " << payloadLen << ", unmarshalling it."); 

    communicationsWatcherFloatingLabelUnmarshal(payload, &lab);

    LabelMessagePtr lm(new LabelMessage);

    lm->label=lab.text;
    lm->fontSize=12;        // not in NodeLabel
    lm->foreground=Color(lab.fgcolor[0], lab.fgcolor[1], lab.fgcolor[2], lab.fgcolor[3]);
    lm->background=Color(lab.bgcolor[0], lab.bgcolor[1], lab.bgcolor[2], lab.bgcolor[3]);
    lm->expiration=lab.expiration;
    lm->addLabel=addLabel;
    lm->lat=lab.x;      // GTL - I don't think these are GPS coords, but pixel positions...
    lm->lng=lab.y;
    lm->alt=lab.z;

    st->client->sendMessage(lm); 
    TRACE_EXIT();
}
void sendFloatinglabelAdd(void *messageHandlerData, const struct MessageInfo *mi)
{
    TRACE_ENTER();
    sendFloatinglabel(messageHandlerData, mi, true); 
    TRACE_EXIT();
}
void sendFloatingLabelRemove(void *messageHandlerData, const struct MessageInfo *mi)
{
    TRACE_ENTER();
    sendFloatinglabel(messageHandlerData, mi, false); 
    TRACE_EXIT();
}

/* This is called by the API when a neighbor node arrives or departs
 * It is defined using communicationsNeighborRegister().
 *
 * the CommunicationsNeighbor * arg is READ ONLY!
 */
void detectorNeighborUpdate(void *data, CommunicationsNeighbor *cn)
{
    TRACE_ENTER();

    detector *st=(detector*)data;

    EdgeMessagePtr em(new EdgeMessage);

    em->node1=boost::asio::ip::address::from_string("127.0.0.1");  // use this node as head
    em->node2=boost::asio::ip::address_v4(cn->addr);
    em->edgeColor=Color::red;       // GTL may want to put this in a configuration file somewhere.
    em->expiration=0;
    em->layer=HIERARCHY_LAYER;

	switch(cn->state)
	{
		case COMMUNICATIONSNEIGHBOR_ARRIVING:
            LOG_INFO("New neighbor " << em->node2 << " arriving. Dist: " << cn->distance << " type: " << cn->type << ((cn->distance==1)?"onehopArrive":""));
            em->addEdge=true;
		break;
		case COMMUNICATIONSNEIGHBOR_UPDATING:
            LOG_INFO("Neighbor " << em->node2 << " update. Dist: " << cn->distance << " type: " << cn->type << 
				( ((cn->distance==1) & (!(cn->type&COMMUNICATIONSNEIGHBOR_WASONEHOP))) ? "onehopArrive" : (((cn->distance>1) && (cn->type&COMMUNICATIONSNEIGHBOR_WASONEHOP)) ? "onehopDepart" : "")) );
            em->addEdge=false;
		break;
		case COMMUNICATIONSNEIGHBOR_DEPARTING:
			LOG_INFO("Neighbor " << em->node2 << " departing.  Dist: " << cn->distance << " type: " << cn->type << ((cn->type&COMMUNICATIONSNEIGHBOR_WASONEHOP) ? "onehopDepart" : ""));
            em->addEdge=false;
		break;
		default:
			LOG_ERROR("Bad neighbor update code!"); 
            TRACE_EXIT_RET("error"); 
            return;
		break;
	}

    st->client->sendMessage(em); 

    TRACE_EXIT(); 
}

static detector *detectorInit(ManetAddr us, const string &serverName, const char *readlog, const char *writelog, 
        CommunicationsMessageDirection direction, unsigned int position, CommunicationsMessageAccess mode)
{
    detector *st;

    // st=(detector*)malloc(sizeof(*st));
    st=new detector;
    st->cs=detectorCommsInit(us,readlog,writelog,"watcherHierarchyClient");
    st->client.reset(new Client(serverName)); 

    if (st->cs==NULL)
        return NULL;

    idsPositionRegister(st->cs, COORDINATOR_ROOT,IDSPOSITION_ACTIVE,myDetectorPositionUpdate,st);
    idsPositionRegister(st->cs, COORDINATOR_REGIONAL,IDSPOSITION_ACTIVE,myDetectorPositionUpdate,st);
    idsPositionRegister(st->cs, COORDINATOR_NEIGHBORHOOD,IDSPOSITION_ACTIVE,myDetectorPositionUpdate,st);

    communicationsNeighborRegister(st->cs,detectorNeighborUpdate,st);

    struct 
    {
        ::MessageType type;
        ::MessageHandler messageHandler;
    } types[] = 
    {
        { IDSCOMMUNICATIONS_MESSAGE_WATCHER_LABEL, &sendLabelAdd },
        { IDSCOMMUNICATIONS_MESSAGE_WATCHER_LABEL_REMOVE, &sendLabelRemove },
        { IDSCOMMUNICATIONS_MESSAGE_WATCHER_COLOR, &sendWatcherColor },
        { IDSCOMMUNICATIONS_MESSAGE_WATCHER_EDGE, &sendEdgeAdd },
        { IDSCOMMUNICATIONS_MESSAGE_WATCHER_EDGE_REMOVE, &sendEdgeRemove },
        { IDSCOMMUNICATIONS_MESSAGE_WATCHER_GPS, &sendGPS },
        { IDSCOMMUNICATIONS_MESSAGE_WATCHER_GRAPH, &sendGraph },
        { IDSCOMMUNICATIONS_MESSAGE_WATCHER_GRAPH_EDGE, &sendGraphEdge },
        { IDSCOMMUNICATIONS_MESSAGE_WATCHER_FLOATINGLABEL, &sendFloatinglabelAdd },
        { IDSCOMMUNICATIONS_MESSAGE_WATCHER_FLOATINGLABEL_REMOVE, &sendFloatingLabelRemove }
    };

    for (unsigned int i = 0; i < sizeof(types)/sizeof(types[0]); i++)
        messageHandlerSet(st->cs, direction, position, mode, types[i].type, types[i].messageHandler, st);

    return st;
}

static void detectorDestroy(detector *st)
{
    CommunicationsLogStatePtr cl = communicationsLogStateGet(st->cs);
    if (st->cs) communicationsClose(st->cs);
    if (cl) communicationsLogClose(cl);
    delete st;
}


#define GETMAXFD(mfd,nfd) mfd=(nfd>mfd)?nfd:mfd

/* Simple select loop to listen on the api FD, and call readReady()
 * when a new message arrives.
 *
 * The API is not threadsafe!
 */
static void selectLoop(detector *dt)
{
    fd_set readfds,writefds;
    int maxfd;
    int rc;
    // struct timeval curtime;
    struct timeval timeout;
    int apifd;
    CommunicationsLogStatePtr cl = communicationsLogStateGet(dt->cs);

    while(1)
    {
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        maxfd=-1;

        apifd=communicationsReturnFD(dt->cs);

        if (apifd>0)
        {
            FD_SET(apifd,&readfds);
            GETMAXFD(maxfd,apifd);
        }

        if (cl)
        {
            /* read events from the log */
            int step = timeout.tv_sec * 1000 + timeout.tv_usec / 1000;
            // fprintf(stderr,"time= %ld.%03ld processing next %dms of events from log\n", curtime.tv_sec, curtime.tv_usec/1000, step);
            if (communicationsLogStep(cl, step, NULL) < 0)
            {
                /* EOF */
                break;
            }
        }
        else
        {
            /* get events from the daemon */
            // fprintf(stderr,"entering select.  timeout= %d\n",timeout.tv_sec);
            rc=select(maxfd+1,&readfds,&writefds,NULL,&timeout);

            if (rc>0)
            {
                if ((apifd>=0) && (FD_ISSET(apifd,&readfds)))
                {
                    // fprintf(stderr,"API fd readable\n");
                    communicationsReadReady(dt->cs);
                }
            }
        }
    }
}

int main(int argc, char *argv[])
{
    detector *dt;
    int ch;
    unsigned int us=0;
    const char *readlog = NULL;
    const char *writelog = NULL;
    CommunicationsMessageDirection direction=COMMUNICATIONS_MESSAGE_INBOUND;
    unsigned int position=COMMUNICATIONS_MESSAGE_AFTERALL;
    CommunicationsMessageAccess mode=COMMUNICATIONS_MESSAGE_READONLY;
    string serverName;

    LOAD_LOG_PROPS("log.properties"); 

    while((ch=getopt(argc, argv,"t:u:w:r:s:?h"))!=-1)
    {
        switch(ch)
        {
            case 'u':
                us=communicationsHostnameLookup(optarg);
                break;
            case 's':
                serverName=optarg;
                break;
            case 'w':
                writelog = optarg;
                break;
            case 'r':
                readlog = optarg;
                break;
            case '?':
            case 'h':
                fprintf(stderr,
                        "%s [options]\n"
                        "options:\n"
                        "-u addr - connect to a daemon on a remote machine\n"
                        "-w file - write API events to the given file for later replay (see -r)\n"
                        "-r file - read API events (e.g., incoming messages) from the given file\n"
                        "          (\"-\" is stdin) instead of connecting to a daemon\n"
                        ,
                        argv[0]);
                exit(1);
        }
    }

    if (!serverName.length())
    {
        fprintf(stderr, "You must specify the watcherd server on the command line with the -s option\n"); 
        TRACE_EXIT_RET(EXIT_FAILURE);
        return EXIT_FAILURE;
    }

    dt=detectorInit(us,serverName,readlog,writelog,direction,position,mode);	/* In a real detector, us=0.  */

    if (dt==NULL)
    {
        fprintf(stderr,"detector init failed, probably could not connect to infrastructure demon.\n");
        exit(1);
    }
    printf("%s: starting\n",argv[0]);
    selectLoop(dt);
    detectorDestroy(dt);
    return 0;
}
