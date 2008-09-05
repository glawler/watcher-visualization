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
#include "transDuo.h"
#include "transformSign.h"
#include "demolib.h"

/*
 *  Copyright (C) 2007  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 *
 * This is an example a detector that signs its reports. This shows how 
 * to use the Infrastructure API along with TransDuo and friends to create 
 * and send signed reports.
 *
 * Every 2 seconds (by default), this detector will send a signed message 
 * to its coordinator.
 */

static const char *rcsid __attribute__ ((unused)) = "$Id: demosigningdetector.c,v 1.9 2007/07/25 20:35:01 dkindred Exp $";

/*
 * For:
 *   ManetAddr foo = getMeAManetAddr();
 *   printf("%d.%d.%d.%d", IPQUAD(foo));
 */
#define IPQUAD(a) \
    ((uint8_t const *)&(a))[3], \
    ((uint8_t const *)&(a))[2], \
    ((uint8_t const *)&(a))[1], \
    ((uint8_t const *)&(a))[0]

/*
 * Detector state that gets passed around
 */
typedef struct 
{
    char const *prog;
    struct TransDuoManager *m;
    CommunicationsStatePtr cs;
    int rootflag;
    int reportperiod;
    Transform *sign;
    struct TransDuo *outbound;
    CommunicationsDestinationType dstType;
} State;

/*
 * non-zero to keep sending
 */
static int globalJustKeepSwimming = 1; 

/*
 * Build a dummy message to pass up the infrastructure
 *
 * Returns zero on success.
 */
static int buildMessage(State *state, int messageId, void **msg_ret, size_t *msglen_ret)
{
    int ret;
    char buff[50];
    xmlChar *mem;
    int size;
    xmlDocPtr doc = xmlNewDoc((unsigned char const*)"1.0");
    if(doc)
    {
        ManetAddr ma = communicationsNodeAddress(state->cs);
        doc->children = xmlNewDocNode( doc, 0, (unsigned char const*)"report", 0);
        sprintf(buff, "%d.%d.%d.%d", IPQUAD(ma));
        xmlSetProp(doc->children, (unsigned char*)"origin", (unsigned char*)buff);
        xmlSetProp(doc->children, (unsigned char*)"type", (unsigned char*)"leaf");
        sprintf(buff, "%u", messageId);
        xmlSetProp(doc->children, (unsigned char*)"num", (unsigned char*)buff);
        xmlDocDumpMemory(doc, &mem, &size);   /* convert xml struct into a xml stream  */
        if(mem)
        {
            if(size > 0)
            {
                // "size" doesn't include the terminating null
                if((ret = transDuoAdd(state->outbound, mem, size + 1, state->sign)) == 0)
                {
                    *msg_ret = transDuoBufferDup(state->outbound);
                    *msglen_ret = transDuoBufferLengthGet(state->outbound);
                }
                else
                {
                    fprintf(stderr, "%s: Failed sign message. \"%s\" (%d)\n",
                            state->prog, strerror(ret), ret);
                    *msglen_ret = 0;
                }
            }
            else
            {
                fprintf(stderr, "%s: Failed to marshal XML doc, marshaled\n"
                        "%s: doc zero characters long\n", state->prog, state->prog);
                *msg_ret = 0;
                *msglen_ret = 0;
                ret = EINVAL;
            }
            xmlFree(mem);
        }
        else
        {
            fprintf(stderr, "%s: Failed to marshal XML doc\n", state->prog);
            *msg_ret = 0;
            *msglen_ret = 0;
            ret = ENOMEM;
        }
        xmlFreeDoc(doc);
    }
    else
    {
        fprintf(stderr, "%s: Failed to create XML doc\n", state->prog);
        *msg_ret = 0;
        *msglen_ret = 0;
        ret = ENOMEM;
    }
    return ret;
} /* buildMessage */

/*
 * This is called regularly by the select loop (below)
 * It will create a message, and send it to this node's coordinator
 */
static void reportSend(State *state)
{
    static int messageId = 0;
    CommunicationsDestination dst = 
    { 
        .addr = NODE_LOCAL, 
        .type = state->dstType, 
        .ttl = 255 
    };
    MessageInfoPtr mi = messageInfoCreate(
            state->cs,
            IDSCOMMUNICATIONS_MESSAGE_DEMO_SIGNED_REPORT,
            dst,
            detectorMessageStatus,
            (void*)messageId);
    if(mi)
    {
        void *payload = NULL;
        size_t payloadLength = 0;
        if(buildMessage(state, messageId, &payload, &payloadLength) == 0)
        {
            printf("%s: sending report %d\n", state->prog, messageId);
            messageInfoRawPayloadSet(mi, payload, payloadLength);
            messageInfoSend(mi);
            messageId++;
        }
        else
        {
            fprintf(stderr, "%s: Failed to build message\n", state->prog);
        }
    }
    else
    {
        fprintf(stderr, "%s: Failed to create MessageInfo\n", state->prog);
    }
    return;
} /* reportSend */

/*
 * Initialize a detector state to pass around
 */
static State *stateCreate(
        char const *prog, 
        ManetAddr us, 
        int reportperiod, 
        char const *keypath,
        CommunicationsDestinationType dstType)
{
    State *state;
    if(keypath == 0)
    {
        keypath = "privatekey.dat";
    }

    state = (State*)malloc(sizeof(*state) + strlen(prog) + 1);
    if(state)
    {
        state->prog = (char const *)(state + 1);
        state->m = transDuoManagerCreate();
        if(state->m)
        {
            strcpy((char *)(state + 1), prog);
            state->reportperiod = reportperiod;
            transDuoInit();
            state->outbound = transDuoCreate(state->m);
            if(state->outbound)
            {
                state->sign = transformSignCreate(keypath);
                if(state->sign)
                {
                    state->cs = communicationsInit(us);
                    if (state->cs)
                    {
                        state->dstType = dstType;
                        communicationsNameSet(state->cs, "demosigningdetector", "");
                        communicationsNeighborRegister(state->cs, detectorNeighborUpdate, state);
                    }
                    else
                    {
                        fprintf(stderr, "%s:  Failed initialize communications, probably could\n"
                                "%s:  not connect to infrastructure daemon \n", prog, prog);
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
                    fprintf(stderr, "%s:  Failed read key file \"%s\"\n", prog, keypath);
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
    transDuoDestroy(state->outbound);
    transDuoManagerDestroy(state->m);
    transDuoFini();
    // destroy the transform after the transDuo that may have a handle
    // that references this transform.
    state->sign->transformDestroy(state->sign);
    free(state);
    return;
}

/*
 * Called when the select loop times out. Sends a report.
 *
 * Sets "*timeout" to "*period" and resets the "*nextreport" to be
 * "*period" from "*curtime".
 */
static void onReportSendTimeout(
        State *state,
        struct timeval const *curtime,
        struct timeval const *period,
        struct timeval *nextreport, 
        struct timeval *timeout)
{
    if (communicationsReturnFD(state->cs) < 0)
    {
        // calls callbacks with error indications
        communicationsReadReady(state->cs);
    }
	reportSend(state);
    *timeout = *period;
    timeradd(curtime, timeout, nextreport);
    return;
} // onReportSendTimeout

/*
 * Checks if a timeout actually occured. If it does, calls
 * "onReportSendTimeout()" which will update both "*nextreport" and
 * "*timeout", otherwise, just updates "*timeout"
 */
static void resetTimeout(
        State *state,
        struct timeval const *period,
        struct timeval *nextreport, 
        struct timeval *timeout)
{ 
    struct timeval curtime;
    gettimeofday(&curtime, 0);
    if(timercmp(&curtime, nextreport, >))
    {
        onReportSendTimeout(state, &curtime, period, nextreport, timeout);
    }
    else
    {
        timersub(nextreport, &curtime, timeout);
    }
    return;
} /* resetTimeout */


/*
 * Simple select loop to listen on the api FD, and break out every so
 * often to send a message.
 *
 * The API is not threadsafe!
 */
static void selectLoop(State *state)
{
    int nfds;
    struct timeval nextreport;
    struct timeval period = 
    { 
        state->reportperiod/1000, 
        (state->reportperiod %1000) * 1000 
    };
    struct timeval timeout = period;
    gettimeofday(&nextreport, 0);
    timeradd(&nextreport, &period, &nextreport);
    while(globalJustKeepSwimming)
    {
        int rc;
        fd_set readfds;
        int apifd = communicationsReturnFD(state->cs);

        FD_ZERO(&readfds);
        if(apifd >= 0)
        {
            FD_SET(apifd, &readfds);
            nfds = apifd + 1;
        }
        else
        {
            nfds = 0;
        }

        rc = select(nfds, &readfds, 0, 0, &timeout);
        if(rc < 0)
        {
            if(errno != EINTR)
            {
                break; // got some more select() failure
            }
            resetTimeout(state, &period, &nextreport, &timeout);
        }
        else if (rc > 0)
        {
            if ((apifd > 0) && (FD_ISSET(apifd, &readfds)))
            {
                // get data and call callbacks
                communicationsReadReady(state->cs);
            }
            resetTimeout(state, &period, &nextreport, &timeout);
        }
        else
        {
            struct timeval curtime;
            gettimeofday(&curtime, 0);
            onReportSendTimeout(state, &curtime, &period, &nextreport, &timeout); 
        }
    }
    return;
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
    printf(" --help|-h           this message\n");
    printf(" --key|-k path       where to find the private key\n");
    printf("                     (defaults to privatekey.dat)\n");
    printf(" --period|-p msec    time between sending alerts\n");
    printf(" --send|-s relation  where to send the report. either\n");
    printf("                     \"parent\" or \"local\" (defaults\n");
    printf("                     to \"local\")\n");
    printf(" --us|-u host        which host to connect to\n");
    printf("                     (defaults to local host)\n");
    printf("                     can be IPv4 or hostname\n");
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
    int reportperiod = 2000;
    char const *keypath = 0;
    int option_index = 0;
    CommunicationsDestinationType dstType = COMMUNICATIONSDESTINATION_DIRECT;

    for(;;)
    {
        static struct option long_options[] = 
        {
            {"help", 1, 0, 'h'},
            {"key", 1, 0, 'k'},
            {"period", 1, 0, 'p'},
            {"send", 1, 0, 's'},
            {"us", 1, 0, 'u'},
            {0, 0, 0, 0}
        };

        int c = getopt_long(argc, argv, "hk:p:s:u:", long_options, &option_index);
        if (c == -1)
            break;

        switch (c)
        {
            case 'k':
                keypath = optarg;
                break;
            case 'p':
                if(atoi(optarg) > 0)
                {
                    reportperiod = atoi(optarg);
                }
                else
                {
                    usage(prog, "period must be greater than 0");
                }
                break;
            case 'u':
            case 's':
                if(strcasecmp(optarg, "parent") == 0)
                {
                    dstType = COMMUNICATIONSDESTINATION_PARENTSOF;
                }
                else if(strcasecmp(optarg, "local") == 0)
                {
                    dstType = COMMUNICATIONSDESTINATION_DIRECT;
                }
                else
                {
                    usage(prog, "--send relation must be \"parent\" or \"local\"");
                }
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
    state = stateCreate(prog, us, reportperiod, keypath, dstType);
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
        fprintf(stderr, "detector init failed.\n");
        ret = 1;
    }
    return ret;
} // main
