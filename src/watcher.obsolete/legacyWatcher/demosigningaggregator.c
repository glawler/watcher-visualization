#include <arpa/inet.h>
#include <errno.h>
#include <getopt.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>


#include "idsCommunications.h"
#include "demolib.h"
#include "transDuo.h"
#include "transformSign.h"
#include "untransformSign.h"

/*  Copyright (C) 2004  Networks Associates Technology, Inc.
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 *
 * This is an example aggregator, to show how to use the Infrastructure API.
 * It listens for messages from the demodetector, and sends aggregated messages
 * every 2 seconds.
 *
 */

static const char *rcsid __attribute__ ((unused)) = "$Id: demosigningaggregator.c,v 1.13 2007/07/21 02:01:58 sherman Exp $";

#define TYPE_NAME ((unsigned char *)"type")
#define NUM_NAME ((unsigned char *)"num")
#define ORIGIN_NAME ((unsigned char *)"origin")

#define IPQUAD(a) \
    ((uint8_t const *)&(a))[3], \
    ((uint8_t const *)&(a))[2], \
    ((uint8_t const *)&(a))[1], \
    ((uint8_t const *)&(a))[0]

typedef struct 
{
    char const *prog;
    struct TransDuoManager *m;
    CommunicationsStatePtr cs;
    xmlDocPtr accumulated;
    int rootflag;
    Transform *sign;
    struct TransDuo *outbound;
    int logOnly;
} State;

static int messageId = 0;
static int globalJustKeepSwimming = !0;

/*
 * Print out "cur" to "fd"
 */
static void childParse(FILE *fd, xmlNodePtr cur, int distance)
{
    static char const *unknown = "?";
    char const *curName = (char const *)cur->name;
    unsigned char *curOriginToFree = xmlGetProp(cur, ORIGIN_NAME);
    char const *curOrigin = curOriginToFree ? (char*)curOriginToFree : unknown;

    fprintf(fd, "xmlnode %s origin %s\n", curName, curOrigin);

    if(strcmp(curName, "report") == 0)
    {
        char *curType = (char*)xmlGetProp(cur, TYPE_NAME);
        if(curType)
        {
            if(strcmp(curType, "leaf") == 0)
            {
                char *curNum = (char*)xmlGetProp(cur, NUM_NAME);
                if(curNum)
                {
                    fprintf(fd, "node %s report %s distance %d\n", curOrigin, curNum, distance);
                    xmlFree(curNum);
                }
                else
                {
                    fprintf(fd, "node %s report ? distance %d\n", curOrigin, distance);
                }
            }
            else if(strcmp(curType, "accumulated") == 0)
            {
                xmlNodePtr child = cur->children;
                while(child)
                {
                    if(strcmp((char const*)child->name, "report") == 0)
                    {
                        childParse(fd, child, distance + 1);
                    }
                    child = child->next;
                }
            }
            xmlFree(curType);
        }
    }
    if(curOriginToFree)
    {
        xmlFree(curOriginToFree);
    }
    return;
} /* childParse */

/*
 * Print out "doc" to "fd"
 */
static void detectorParse(FILE *fd, xmlDocPtr doc)
{
    xmlNodePtr p, cur;

    cur = xmlDocGetRootElement(doc);

    p=cur;
    while(p)
    {
        printf("xmlnode %s \n", p->name);
        childParse(fd, p, 0);
        p=p->next;
    }
    return;
} /* detectorParse */

/* 
 * copy all the children of child to node 
 */ 
static void childAdd(State *state, xmlNodePtr node, xmlNodePtr child)   
{
    unsigned char *propertyName[] =
    {
        TYPE_NAME, NUM_NAME, ORIGIN_NAME
    };

    while(child)
    {
        size_t i;
        xmlNodePtr nextNod = xmlNewChild(node, 0, (unsigned char*)"report", 0);
        if(nextNod)
        {
            node = nextNod;
            for(i = 0; i < sizeof(propertyName)/sizeof(propertyName[0]); ++i)
            {
                unsigned char *property = xmlGetProp(child, propertyName[i]);
                if(property)
                {
                    xmlSetProp(node, propertyName[i], property);
                    xmlFree(property);
                }
                else
                {
                    fprintf(stderr, "%s: Failed xmlGetProp(\"%s\")\n",
                            state->prog, propertyName[i]);
                }
            }
        }
        else
        {
            fprintf(stderr, "%s: Failed xmlNewChild\n", state->prog);
        }
        childAdd(state, node, child->children);
        child = child->next;
    }
    return;
} /* childAdd */

/* This is called by the API when a message arrives
 * It is defined using the API function messageHandlerSet(), which also takes
 * the type of message as an argument.
 *
 * It is expected that a detector will have a separate function for each message
 * type that it can handle.  However if a detector wishes to use the same one, the
 * message type can be specified to the callback by using a field in the detector
 * defined structure pointed to by the data pointer.  The value of that pointer
 * is also an argument to messageHandlerSet().
 *
 * In this case, the void pointer is used to point to the detector state structure,
 * so it can access the accumulated messages.
 *
 */
static void onReportArrive(void *data, const struct MessageInfo *mi)
{
    State *state = (State*)data;
    ManetAddr originator = messageInfoOriginatorGet(mi);
    struct TransDuo *inbound;

    printf("%s: received a report from %d\n",
	   state->prog, originator & 0xFF);

    // parse and verify
    inbound = transDuoFromBuffer(
            state->m,
            messageInfoRawPayloadGet(mi),
            messageInfoRawPayloadLenGet(mi));

    if(inbound)
    {
        printf("%s: transDuoFromBuffer succeeded, raw len=%d\n",
	       state->prog, transDuoRawLengthGet(inbound));

        // make sure the message we got was signed the way we want
        if((uint32_t)transDuoTagGet(inbound) == state->sign->tag)
        {
            ManetAddr signerMa = untransformSignSigner(transDuoUntransformDataHandleGet(inbound));
            printf("%s: Message signed by %d.%d.%d.%d\n",
                    state->prog, IPQUAD(signerMa));
            if (originator != signerMa)
            {
                printf("%s: Message originated by %d.%d.%d.%d, but signed by %d.%d.%d.%d\n",
                    state->prog, IPQUAD(originator), IPQUAD(signerMa));
            }
            printf("%s: payload:\n", state->prog);
            fwrite(transDuoRawGet(inbound), 1, transDuoRawLengthGet(inbound), stdout);

            // get the verified buffer into an xmlDocPtr
            xmlDocPtr report = xmlParseMemory(
                    (const char*)transDuoRawGet(inbound),
                    transDuoRawLengthGet(inbound));
            if(report)
            {
                char *rootTypeProperty = (char*)xmlGetProp(xmlDocGetRootElement(report), TYPE_NAME);
                if(rootTypeProperty)
                {
                    CommunicationsNeighbor *neigh = communicationsNeighborSearch(state->cs, originator);
                    int isChild = neigh && neigh->type & COMMUNICATIONSNEIGHBOR_CHILD;
                    if(isChild || strcmp(rootTypeProperty, "leaf") == 0)
                    {
                        detectorParse(stdout, report);
                        if(!state->logOnly)
                        {
                            if(state->accumulated == 0)	/* do we need to make a new report?   */
                            {
                                ManetAddr nodeMa  = communicationsNodeAddress(state->cs);
                                state->accumulated = xmlNewDoc((unsigned char*)"1.0");
                                if(state->accumulated)
                                {
                                    unsigned char buff[50];
                                    state->accumulated->children = 
                                        xmlNewDocNode(
                                                state->accumulated, 
                                                0, 
                                                (unsigned char*)"report", 
                                                0);
                                    sprintf((char*)buff, "%d.%d.%d.%d", IPQUAD(nodeMa));
                                    xmlSetProp(
                                            state->accumulated->children, 
                                            ORIGIN_NAME, 
                                            buff);
                                    xmlSetProp(
                                            state->accumulated->children, 
                                            TYPE_NAME, 
                                            (unsigned char*)"accumulated");
                                    childAdd(state, state->accumulated->children, report->children);
                                }
                                else
                                {
                                    fprintf(stderr, "%s: Failed xmlNewDoc\n", state->prog);
                                }
                            }
                            else
                            {
                                childAdd(state, state->accumulated->children, report->children);
                            }
                        }
                    }
                    else
                    {
                        fprintf(stderr, "refused a message from from %d  len= %d  payload:\n",
                                messageInfoOriginatorGet(mi) & 0xFF,
                                messageInfoRawPayloadLenGet(mi));
                    }
                    xmlFree(rootTypeProperty);
                }
                else
                {
                    fprintf(stderr, "%s: Failed to get the \"type\" property of the \n"
                            "%s: root element of a report from %d.%d.%d.%d\n",
                            state->prog, state->prog, IPQUAD(originator));
                }
                xmlFreeDoc(report);
            }
            else
            {
                fprintf(stderr, "%s: Failed to parse a report from %d.%d.%d.%d\n", 
                        state->prog, IPQUAD(originator));
            }
        }
        else
        {
            fprintf(stderr, "%s: Invalid message type from %d.%d.%d.%d\n" 
                    "%s: Expected message tag %d, got %d\n",
                    state->prog, IPQUAD(originator),
                    state->prog, state->sign->tag, transDuoTagGet(inbound));
        }
        // clean the "struct TransDuo*" from "transformDuoFromBuffer()"
        transDuoDestroy(inbound);
    }
    else
    {
        fprintf(stderr, "%s: Failed to read a signed report from %d.%d.%d.%d\n", 
                state->prog, IPQUAD(originator));
    }
    return;
} /* onReportArrive */

/* Called when we get a directive.  
 * we then forward the directive to our children
 */
static void onDirectiveArrive(void *data, const struct MessageInfo *mi)
{
    State *state = (State*)data;
    ManetAddr originator = messageInfoOriginatorGet(mi);
    // parse and verify the signed buffer
    struct TransDuo *inbound = transDuoFromBuffer(
            state->m,
            messageInfoRawPayloadGet(mi),
            messageInfoRawPayloadLenGet(mi));
    if(inbound)
    {
        // make sure the message we got was signed the way we want
        if((uint32_t)transDuoTagGet(inbound) == state->sign->tag)
        {
            static const CommunicationsDestination dst = 
            { 
                .addr = NODE_LOCAL, 
                .type = COMMUNICATIONSDESTINATION_CHILDRENOF, 
                .ttl = 255 
            };
            MessageInfoPtr newmi = messageInfoCreate(
                    state->cs, 
                    IDSCOMMUNICATIONS_MESSAGE_DEMO_DIRECTIVE, 
                    dst, 
                    detectorMessageStatus, 
                    (void*)messageId);
            printf("%s: got a directive. \"%s\" from %d.%d.%d.%d\n", 
                    state->prog, (char const *)transDuoRawGet(inbound), IPQUAD(originator));
            if(newmi)
            {
                /*
                 * state->sign will not actually re-sign the directive but
                 * it does know how to reformat the signed directive that
                 * came in for re-sending.
                 */
                transDuoCopy(state->outbound, inbound, state->sign);
                messageInfoRawPayloadSet(
                        newmi, 
                        transDuoBufferDup(state->outbound),
                        transDuoBufferLengthGet(state->outbound));
                printf("%s: forwarding directive as msg %d\n", state->prog, messageId);
                messageInfoSend(newmi);
                messageId++;
            }
            else
            {
                fprintf(stderr, "%s: Failed to create a MessageInfo for message from %d.%d.%d.%d \n", 
                        state->prog, IPQUAD(originator));
            }
        }
        else
        {
                fprintf(stderr, "%s: Invalid message type from %d.%d.%d.%d\n" 
                                "%s: Expected message tag %d, got %d\n",
                        state->prog, IPQUAD(originator),
                        state->prog, state->sign->tag, transDuoTagGet(inbound));
        }
        transDuoDestroy(inbound);
    }
    else
    {
        fprintf(stderr, "%s: Failed to read a signed report from %d.%d.%d.%d\n", 
                state->prog, IPQUAD(originator));
    }
    return;
} /* onDirectiveArrive */

/* This is called by the API when this node's position in the hierarchy changes
 * It is defined using the API function idsPositionRegister().
 */
static void myDetectorPositionUpdate(void *data, IDSPositionType position, IDSPositionStatus status)
{
    State *state = (State*)data;

    if (position == COORDINATOR_ROOT)
    {
        state->rootflag = (status == IDSPOSITION_ACTIVE);
    }

    detectorPositionUpdate(data, position, status);
}


/* This is called regularly by the select loop (below)
 * It will create a message, and send it to this node's coordinator
 */
static void aggregatorSend(State *state)
{
    /* If we have any accumulated messages, forward them to our coordinator */
    if (state->accumulated)
    {
        xmlChar *mem;
        int size;
        /* convert xml struct into a xml stream  */
        xmlDocDumpMemory(state->accumulated, &mem, &size);
        if(mem)
        {
            static const CommunicationsDestination dst = 
            {
                .addr = NODE_LOCAL,
                .type = COMMUNICATIONSDESTINATION_PARENTSOF,
                .ttl = 255
            };
            MessageInfoPtr mi;
            // Sign and append the signature to "(mem, size+1)"
            // (xmlDocDumpMemory "size" doesn't include the trailing null)
            transDuoAdd(state->outbound, mem, size + 1, state->sign);
            printf("sending aggregated report %d  payload= %s.\n", 
                    messageId, mem);
            mi = messageInfoCreate(
                    state->cs, 
                    IDSCOMMUNICATIONS_MESSAGE_DEMO_REPORT, 
                    dst, 
                    detectorMessageStatus, 
                    (void*)messageId);
            if(mi)
            {
                // copy the marhalled version of the signed buffer into
                // the MessageInfo payload
                messageInfoRawPayloadSet(
                        mi, 
                        transDuoBufferDup(state->outbound),
                        transDuoBufferLengthGet(state->outbound));
                messageInfoSend(mi);
                messageId++;
                xmlFree(mem);
            }
            else
            {
                fprintf(stderr, "%s: Failed messageInfoCreate\n", state->prog);
            }
        }
        else
        {
            fprintf(stderr, "%s: Failed to get XML document stream\n", state->prog);
        }
        xmlFreeDoc(state->accumulated);
        state->accumulated=0;
    }
    return;
} /* aggregatorSend */

/*
 * Initialize a aggregator state to pass around
 */
static State *stateCreate(
        char const *prog, 
        ManetAddr us, 
        char const *privpath, 
        char const *pubpath,
        int logOnly,
        MessageType reportMessageType)
{
    State *state;
    if(privpath == 0)
    {
        privpath = "privatekey.dat";
    }
    if(pubpath == 0)
    {
        pubpath = "publickey.dat";
    }

    state = (State*)malloc(sizeof(*state) + strlen(prog) + 1);
    if(state)
    {
        // Initialize internal TransDuo processing data
        transDuoInit();
        // Get a manager to manage TransDuos
        state->m = transDuoManagerCreate();
        if(state->m)
        {
            state->prog = (char const *)(state + 1);
            strcpy((char *)(state + 1), prog);
            state->accumulated = 0;
            // Create a TransDuo for outbound MessageInfo payloads
            state->outbound = transDuoCreate(state->m);
            if(state->outbound)
            {
                // Create a Transform for signing
                state->sign = transformSignCreate(privpath);
                if(state->sign)
                {
                    // Create a Untransform for signing (does verify and
                    // strips the signature).
                    Untransform *u = untransformSignCreate(pubpath);
                    if(u)
                    {
                        // Register the Untransform with the TransDuo
                        // processing code (so transDuoFromBuffer() can find
                        // the proper Untransform to use).
                        transDuoUntransformRegister(state->m, u); // TransDuo owns "u"
                        state->cs = communicationsInit(us);
                        if (state->cs)
                        {
                            state->logOnly = logOnly;
                            communicationsNameSet(state->cs, "demosigningaggregator", "");
                            // can be root
                            idsPositionRegister(
                                    state->cs, 
                                    COORDINATOR_ROOT, 
                                    IDSPOSITION_ACTIVE, 
                                    myDetectorPositionUpdate, state);
                            // can be regional coordinator
                            idsPositionRegister(
                                    state->cs, 
                                    COORDINATOR_REGIONAL, 
                                    IDSPOSITION_ACTIVE, 
                                    detectorPositionUpdate, 
                                    state);
                            // can be neighborhood coordinator
                            idsPositionRegister(
                                    state->cs, 
                                    COORDINATOR_NEIGHBORHOOD, 
                                    IDSPOSITION_ACTIVE, 
                                    detectorPositionUpdate, state);
                            // show neighbor changes
                            communicationsNeighborRegister(
                                    state->cs, 
                                    detectorNeighborUpdate, state);
                            // get signed reports
                            fprintf(stderr, "%s: Subscribing to message type 0x%x\n", prog, reportMessageType);
                            messageHandlerSet(
                                    state->cs, 
                                    COMMUNICATIONS_MESSAGE_INBOUND, 
                                    COMMUNICATIONS_MESSAGE_AFTERALL, 
                                    COMMUNICATIONS_MESSAGE_READONLY, 
                                    reportMessageType, 
                                    onReportArrive, state);
                            // get signed directives
                            fprintf(stderr, "%s: Subscribing to message type 0x%x\n", prog, IDSCOMMUNICATIONS_MESSAGE_DEMO_SIGNED_DIRECTIVE);
                            messageHandlerSet(
                                    state->cs, 
                                    COMMUNICATIONS_MESSAGE_INBOUND, 
                                    COMMUNICATIONS_MESSAGE_AFTERALL, 
                                    COMMUNICATIONS_MESSAGE_READONLY, 
                                    IDSCOMMUNICATIONS_MESSAGE_DEMO_SIGNED_DIRECTIVE, 
                                    onDirectiveArrive, state);
                        }
                        else
                        {
                            fprintf(stderr, "%s:  Failed initialize communications, probably could \n", prog);
                            fprintf(stderr, "%s:  not connect to infrastructure daemon \n", prog);
                            state->sign->transformDestroy(state->sign);
                            transDuoDestroy(state->outbound);
                            transDuoManagerDestroy(state->m);
                            transDuoFini();
                            free(state);
                            state = 0;
                        }
                    }
                    else
                    {
                        fprintf(stderr, "%s:  Failed public key pool \"%s\"\n", prog, pubpath);
                        state->sign->transformDestroy(state->sign);
                        transDuoDestroy(state->outbound);
                        transDuoManagerDestroy(state->m);
                        transDuoFini();
                        free(state);
                        state = 0;
                    }
                }
                else
                {
                    fprintf(stderr, "%s:  Failed private key file \"%s\"\n", prog, pubpath);
                    transDuoDestroy(state->outbound);
                    transDuoManagerDestroy(state->m);
                    transDuoFini();
                    free(state);
                    state = 0;
                }
            }
            else
            {
                fprintf(stderr, "%s: Failed to create TransDuo for outbound\n", prog);
                transDuoManagerDestroy(state->m);
                transDuoFini();
                free(state);
                state = 0;
            }
        }
        else
        {
            fprintf(stderr, "%s: Failed to create TransDuoManager\n", prog);
            transDuoFini();
            free(state);
            state = 0;
        }
    }
    else
    {
        fprintf(stderr, "%s: failed to allocate %d byte detector state\n", 
                prog, sizeof(*state) + strlen(prog) + 1);
    }
    return state;
} /* stateCreate */

/*
 * Cleanup what was done in stateCreate
 */
static void stateDestroy(State *state)
{
    // Clean up the TransDuo created in "stateCreate()"
    transDuoDestroy(state->outbound);
    // Clean up the TransDuoManager created in "stateCreate()"
    transDuoManagerDestroy(state->m);
    // Clean up the internal TransDuo processing data
    transDuoFini();
    // Clean up the signing transform. Destroy the transform after 
    // the transDuo that may have a handle that references this 
    // transform.
    state->sign->transformDestroy(state->sign);
    if(state->accumulated)
    {
        xmlFreeDoc(state->accumulated);
    }
    free(state);
    return;
} /* stateDestroy */


#define GETMAXFD(mfd, nfd) mfd=(nfd>mfd)?nfd:mfd

/* Simple select loop to listen on the api FD, and break out every 2 seconds to
 * send messages.
 *
 * The API is not threadsafe!
 */
static void selectLoop(State *dt)
{
    fd_set readfds, writefds;
    int maxfd;
    int rc;
    struct timeval nextreport, curtime;
    struct timeval timeout;
    int apifd;

    gettimeofday(&nextreport, 0);
    nextreport.tv_sec += 2;
    while(globalJustKeepSwimming)
    {
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        maxfd=-1;

        apifd=communicationsReturnFD(dt->cs);
        if (apifd>0)
        {
            FD_SET(apifd, &readfds);
            GETMAXFD(maxfd, apifd);
        }

	fflush(stdout);
	fflush(stderr);

        gettimeofday(&curtime, 0);
        if (timercmp(&curtime, &nextreport, >))
        {
            if (apifd<0)
                communicationsReadReady(dt->cs);

            aggregatorSend(dt);
            timeout.tv_sec=2;
            timeout.tv_usec=0;
            timeradd(&curtime, &timeout, &nextreport);
        }
        timersub(&nextreport, &curtime, &timeout);
        rc=select(maxfd+1, &readfds, &writefds, 0, &timeout);

        if (rc>0)
        {
            if ((apifd>0) && (FD_ISSET(apifd, &readfds)))
            {
                communicationsReadReady(dt->cs);
            }
        }
    }
} /* selectLoop */

/*
 * Print a usage statement and exit
 */
static void usage(char const *prog, char const *msg)
{
    if(msg && *msg)
    {
        printf("%s: %s\n", prog, msg);
    }
    printf("usage: %s [options]\n", prog);
    printf("Options:\n");
    printf(" --help|-h                       this message\n");
    printf(" --key|-k path                   where to find the private key\n");
    printf(" --log-only|-l                   Don't aggregate and send reports,\n");
    printf("                                 just log them\n");
    printf(" --report-message-type|-r type   what message class to listen/send\n");
    printf("                                 on for reports (defaults to 0x10045)\n");
    printf(" --pool|-p path                  where to find the public key\n");
    printf("                                 pool. (defaults to publickey.dat\n");
    printf(" --us|-u host                    which host to connect to\n");
    printf("                                 (defaults to local host)\n");
    printf("                                 can be IPv4 or hostname\n");
    exit(1);
}

/*
 * When the program is interrupted, tell the select loop to end.
 */
static void handleSIGINT(int sig)
{
    globalJustKeepSwimming = 0;
    return;
}

int main(int argc, char *argv[])
{
    int ret;
    char const *prog = 
        strrchr(argv[0], '/') ? 
        strrchr(argv[0], '/') + 1 : argv[0];
    State *state;
    ManetAddr us = 0;
    int logOnly = 0;
    MessageType reportMessageType = IDSCOMMUNICATIONS_MESSAGE_DEMO_SIGNED_REPORT;
    char const *privpath = 0;
    char const *pubpath = 0;
    int option_index = 0;

    for(;;)
    {
        static struct option long_options[] = 
        {
            {"help", 1, 0, 'h'},
            {"key", 1, 0, 'k'},
            {"log-only", 1, 0, 'l'},
            {"pool", 1, 0, 'p'},
            {"report-message-type", 1, 0, 'r'},
            {"us", 1, 0, 'u'},
            {0, 0, 0, 0}
        };

        int c = getopt_long(argc, argv, "hk:lp:r:u:", long_options, &option_index);
        if (c == -1)
            break;

        switch (c)
        {
            case 'k':
                privpath = optarg;
                break;
            case 'l':
                logOnly = !0;
                break;
            case 'r':
                reportMessageType = strtol(optarg, NULL, 0);
                break;
            case 'p':
                pubpath = optarg;
                break;
            case 'u':
                us = communicationsHostnameLookup(optarg);
                break;
            case 'h':
            case '?':
                {
                    usage(prog, "");
                }
                break;
        }
    }
    if(optind != argc)
    {
        usage(prog, "");
    }

    /*
     * us == 0 is for the local infrastructure daemon
     */
    state = stateCreate(prog, us, privpath, pubpath, logOnly, reportMessageType);
    if (state)
    {
        printf("%s: starting\n", prog);
        signal(SIGINT, handleSIGINT);
        selectLoop(state);
        stateDestroy(state);
        ret = 0;
    }
    else
    {
        fprintf(stderr, "State init failed.\n");
        ret = 1;
    }
    return ret;
} // main
