#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "idsCommunications.h"

static const char *rcsid __attribute__ ((unused)) = "$Id: watcherpropertytest.c";

/* Test program for modifing things in the watcher.
 *
 *  Copyright (C) 2008  Sparta Inc.  Written by the NIP group, SRD, ISSO
 */

void usage(void);

int main(int argc, char *argv[])
{
    CommunicationsStatePtr cs;
    ManetAddr attach=0;

    int ch;
    char *endPtr=NULL;
    WatcherPropertyInfo prop;
    int shapeSet=0, colorSet=0, effectSet=0, sizeSet=0;

    prop.identifier=0;

    while ((ch = getopt(argc, argv, "d:n:p:s:c:e:z:hH?")) != -1)
        switch (ch)
        {
            case 'n':
                prop.identifier=communicationsHostnameLookup(optarg);
                break;
            case 'd':
                attach=communicationsHostnameLookup(optarg);
                break;
            case 'p':
                {
                    if (0==strcmp("shape", optarg)) prop.property=WATCHER_PROPERTY_SHAPE; 
                    else if (0==strcmp("color", optarg)) prop.property=WATCHER_PROPERTY_COLOR; 
                    else if (0==strcmp("effect", optarg)) prop.property=WATCHER_PROPERTY_EFFECT;
                    else if (0==strcmp("size", optarg)) prop.property=WATCHER_PROPERTY_SIZE;
                    else 
                    {
                        fprintf(stderr, "Error parsing property %s, must be one of 'shape', 'color', 'effect', or 'size'i, or 'size'.\n", optarg);
                        exit(EXIT_FAILURE);
                    }
                }
                break;
            case 's':
                {
                    if (0==strcmp("circle", optarg)) prop.data.shape=WATCHER_SHAPE_CIRCLE; 
                    else if (0==strcmp("square", optarg)) prop.data.shape=WATCHER_SHAPE_SQUARE;
                    else if (0==strcmp("triangle", optarg)) prop.data.shape=WATCHER_SHAPE_TRIANGLE;
                    else if (0==strcmp("torus", optarg)) prop.data.shape=WATCHER_SHAPE_TORUS;
                    else if (0==strcmp("teapot", optarg)) prop.data.shape=WATCHER_SHAPE_TEAPOT;
                    else 
                    {
                        fprintf(stderr, "Error parsing shape %s, must be one of 'circle', 'sqaure', 'triangle', 'torus', or 'teapot'\n", optarg);
                        exit(EXIT_FAILURE);
                    }
                    shapeSet=1;
                }
                break;
            case 'e':
                {
                    if (0==strcmp("spin", optarg)) prop.data.effect=WATCHER_EFFECT_SPIN; 
                    else if (0==strcmp("sparkle", optarg)) prop.data.effect=WATCHER_EFFECT_SPARKLE;
                    else if (0==strcmp("flash", optarg)) prop.data.effect=WATCHER_EFFECT_FLASH;
                    else 
                    {
                        fprintf(stderr, "Error parsing effect %s, must be one of 'spin', 'sparkle', or 'flash'\n", optarg);
                        exit(EXIT_FAILURE);
                    }
                    effectSet=1;
                }
                break;
            case 'c':
                {
                    int tmpcolor;
                    tmpcolor=ntohl(inet_addr(optarg));
                    prop.data.color[0]=tmpcolor >> 24;
                    prop.data.color[1]=tmpcolor >> 16;
                    prop.data.color[2]=tmpcolor >> 8;
                    prop.data.color[3]=tmpcolor;
                    colorSet=1;
                }
                break;
            case 'z':
                {
                    prop.data.size=strtod(optarg, &endPtr);   // strtof() requires _XOPEN_SOURCE >= 600 || _ISOC99_SOURCE; or cc -std=c99, so just use strtod()
                    if(errno == ERANGE || *endPtr)
                    { 
                        fprintf(stderr, "\nI don't understand the size argument: %s\n\n", optarg); 
                        exit(EXIT_FAILURE); 
                    } 
                    sizeSet=1;
                }
                break;
            case '?':
            case 'h':
            case 'H':
            default:
                usage();
                exit(EXIT_FAILURE); 
                break;
        }

    if ((prop.property==WATCHER_PROPERTY_SHAPE && !shapeSet) || 
        (prop.property==WATCHER_PROPERTY_COLOR && !colorSet) || 
        (prop.property==WATCHER_PROPERTY_EFFECT && !effectSet) || 
        (prop.property==WATCHER_PROPERTY_SIZE && !sizeSet))
    {
        fprintf(stderr, "\nInvalid combination of property and data\n\n"); 
        usage();
        exit(EXIT_FAILURE);
    }

    cs=communicationsInit(attach);
    communicationsNameSet(cs,"watcherpropertytest","");

    if (cs==NULL)
    {
        fprintf(stderr,"communicationsInit() failed\n");
        exit(EXIT_FAILURE);
    }

    communicationsWatcherProperty(cs,prop.identifier,&prop);

    communicationsClose(cs);
    return EXIT_SUCCESS;
}

void usage(void)
{
    fprintf(stderr,"Set properties of things in the watcher\n\n"
            "watcherpropertytest -p prop [prop args]\n"
            "\t-d IPaddr - node to connect to (duck)\n"
            "\t-n IPaddr - node to affect\n"
            "\t-p property - one of 'shape', 'color', 'effect', or 'size'\n"
            "\tIf property is shape:\n"
            "\t\t-s one of 'circle', 'square', 'triangle', 'torus', or 'teapot'\n"
            "\t\t(teapot is only drawn in 3d mode)\n"
            "\tIf property is color:\n"
            "\t\t-c red.green.blue.alpha - color to apply\n"
            "\tIf property is effect:\n"
            "\t\t-e - one of 'spin', 'sparkle', or 'flash'\n" 
            "\tIf property is size:\n"
            "\t\t-z size\n");
}

