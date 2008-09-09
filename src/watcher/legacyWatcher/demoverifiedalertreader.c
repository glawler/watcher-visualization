#include <arpa/inet.h>
#include <ctype.h>
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
#include "bufferPair.h"
#include "demolib.h"

/*  
 *  Copyright (C) 2007  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 *
 * This is an example verified alert reader showing how to use the
 * Infrastructure API.  It listens for messages from the alert
 * consolidator, and shows the results.
 *
 */

static const char *rcsid __attribute__ ((unused)) = "$Id: demoverifiedalertreader.c,v 1.3 2007/07/25 13:48:26 dkindred Exp $";

#define IPQUAD(a) \
    ((uint8_t const *)&(a))[3], \
    ((uint8_t const *)&(a))[2], \
    ((uint8_t const *)&(a))[1], \
    ((uint8_t const *)&(a))[0]

#define TYPE_NAME ((unsigned char *)"type")
#define NUM_NAME ((unsigned char *)"num")
#define ORIGIN_NAME ((unsigned char *)"origin")

typedef struct 
{
    char const *prog;
    CommunicationsStatePtr cs;
} State;

static int globalJustKeepSwimming = !0;

/*
 * A hex dump
 */
static void dump(char const *title, uint8_t const *buf, size_t len)
{
    size_t i = 1;
    uint8_t const *c = buf;
    uint8_t const *cend;
    char line[40];
    char line2[20];
    char *p = line;
    char *p2 = line2;
    cend = c + len;
    printf("%s: dumping %d byte buffer\n", title, len);
    while(c != cend)
    {
        p += sprintf(p, "%02x", *c);
        p2 += sprintf(p2, "%c", isprint(*c) ? *c : '.');
        if((i % 16) == 0)
        {
            printf("%s: %s %s\n", title, line, line2);
            p = line;
            p2 = line2;
        } else if((i % 4) == 0) {
            p += sprintf(p, " ");
            if((i % 8) == 0) {
                p2 += sprintf(p2, "-");
            }
        }
        ++i;
        ++c;
    }
    if((i - 1) % 16)
    {
        printf("%s: %-35.35s %s\n", title, line, line2);
    }
    return;
} /* dump */

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

/*
 * This is called by the API when
 * a IDSCOMMUNICATIONS_MESSAGE_IDMEF_ALERT_VERIFIED arrives.
 * It is defined using the API function messageHandlerSet(), which also takes
 * the type of message as an argument.
 */
static void onVerifiedAlertArrive(void *data, const struct MessageInfo *mi)
{
    State *state = (State*)data;
    ManetAddr originator = messageInfoOriginatorGet(mi);
    struct BufferPair *bp;
    ManetAddr thisHost = communicationsNodeAddress(state->cs);

    if (thisHost != originator)
    {
        fprintf(stderr, "%s: WARNING: received a verified alert from non-local host %d.%d.%d.%d -- ignoring.\n", 
                state->prog, IPQUAD(originator));
        return;
    }
    printf("%s: received a verified alert from local host\n",
	   state->prog);

    // parse and verify
    bp = bufferPairFromMarshaled(
            messageInfoRawPayloadGet(mi),
            messageInfoRawPayloadLenGet(mi));

    if(bp)
    {
        uint8_t const *signedBuf = (uint8_t const*)bufferPairFirst(bp);
        size_t signedBufLength = bufferPairFirstLength(bp);
        size_t docTextLength = bufferPairSecondLength(bp);
        xmlChar *docText = (xmlChar*)malloc(docTextLength + 1);
        dump("signed", signedBuf, signedBufLength);
        if(docText)
        {
            xmlChar* xmlTxt;
            int xmlTxtLen;
            memcpy(docText, bufferPairSecond(bp), docTextLength);
            docText[docTextLength] = 0;
            xmlDocPtr doc = 
                xmlReadDoc(
                        docText, 
                        "", 
                        0, 
                        XML_PARSE_PEDANTIC | XML_PARSE_NOBLANKS | XML_PARSE_NONET);
            if (doc)
            {
                printf("%s: received an XML alert\n", state->prog);
                xmlDocDumpFormatMemory(doc, &xmlTxt, &xmlTxtLen, 1);
                printf("%*.*s\n", xmlTxtLen, xmlTxtLen, xmlTxt);
                xmlFree(xmlTxt);
                free(docText);
            }
            else
            {
                printf("%s: alert does not parse as XML\n", state->prog);
            }
        }
    }
    else
    {
        fprintf(stderr, "%s: Failed to read a signed alert from %d.%d.%d.%d\n", 
                state->prog, IPQUAD(originator));
    }
    return;
} /* onVerifiedAlertArrive */

/* This is called by the API when this node's position in the hierarchy changes
 * It is defined using the API function idsPositionRegister().
 */
static void onDetectorPositionUpdate(void *data, IDSPositionType position, IDSPositionStatus status)
{
    detectorPositionUpdate(data, position, status);
}

/*
 * Initialize a aggregator state to pass around
 */
static State *stateCreate(
        char const *prog, 
        ManetAddr us)
{
    State *state;

    state = (State*)malloc(sizeof(*state) + strlen(prog) + 1);
    if(state)
    {
        state->prog = (char const *)(state + 1);
        strcpy((char *)(state + 1), prog);
        state->cs = communicationsInit(us);
        if (state->cs)
        {
            communicationsNameSet(state->cs, "demoverifiedalertreader", "");
            // can be root
            idsPositionRegister(
                    state->cs, 
                    COORDINATOR_ROOT, 
                    IDSPOSITION_ACTIVE, 
                    onDetectorPositionUpdate, state);
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
            // get verified alerts
            messageHandlerSet(
                    state->cs, 
                    COMMUNICATIONS_MESSAGE_INBOUND, 
                    COMMUNICATIONS_MESSAGE_AFTERALL, 
                    COMMUNICATIONS_MESSAGE_READONLY, 
                    IDSCOMMUNICATIONS_MESSAGE_IDMEF_ALERT_VERIFIED,
                    onVerifiedAlertArrive, state);
        }
        else
        {
            fprintf(stderr, "%s:  Failed initialize communications, probably could \n", prog);
            fprintf(stderr, "%s:  not connect to infrastructure daemon \n", prog);
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
    int option_index = 0;

    for(;;)
    {
        static struct option long_options[] = 
        {
            {"help", 1, 0, 'h'},
            {"us", 1, 0, 'u'},
            {0, 0, 0, 0}
        };

        int c = getopt_long(argc, argv, "hu:", long_options, &option_index);
        if (c == -1)
            break;

        switch (c)
        {
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
    state = stateCreate(prog, us);
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
