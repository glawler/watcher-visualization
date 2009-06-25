#include <string>

#include "watcherPropertyData.h"
#include "singletonConfig.h"

#include <sys/socket.h>     // for inet_ntoa
#include <netinet/in.h>     // for inet_ntoa
#include <arpa/inet.h>      // for inet_ntoa

#include <string.h> // for memset()

using namespace std;
using namespace watcher;
using namespace libconfig;

INIT_LOGGER(WatcherPropertyData, "WatcherPropertyData"); 

WatcherPropertyData::WatcherPropertyData()
{
    TRACE_ENTER();
    shape= WATCHER_SHAPE_CIRCLE; 
    sparkle=0;
    spin=0;
    flash=0;
    size=1.0;

    nextSpinUpdate=0;    
    spinRotation_x=0;
    spinRotation_y=0;
    spinRotation_z=0;

    nextFlashUpdate=0;    
    isFlashed=0;

    memset(guiLabel, 0, sizeof(guiLabel));

    TRACE_EXIT();
}

WatcherPropertyData::~WatcherPropertyData()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

WatcherPropertyData *watcher::findWatcherPropertyData(unsigned int index, WatcherPropertiesList &theList)
{
    // TRACE_ENTER();
    WatcherPropertiesList::iterator i;
    for (i=theList.begin(); i!=theList.end(); i++)
    {
        // LOG_DEBUG("Comparing " << index << " to " << (*i)->identifier); 
        if ( (*i)->identifier==index)
        {
            // TRACE_EXIT_RET("found property data");
            return *i;
        }
    }
    // TRACE_EXIT_RET(0);

    // exit(1);
    return NULL;
}

void watcher::loadWatcherPropertyData(WatcherPropertyData *propp, int nodeId)
{
    TRACE_ENTER();

    // Example cfg:
    // watcherProperties:
    // {
    //     node_192_128_1_121:
    //     {
    //          label = "foobar";
    //          shape = "square"; 
    //     };
    // };
    //
    singletonConfig &sc=singletonConfig::instance();
    sc.lock();
    Config &cfg=sc.getConfig(); 

    // Assumes identifier is an inet address - which it might not be. 
    // Currently all identifiers are the node's address, so it's OK for now, 
    // but if things other than node ever get properties, this will have to be
    // changed. 
    string idAsAddr("node_");
    struct in_addr tmpAddr;
    tmpAddr.s_addr=htonl(propp->identifier);
    idAsAddr+=inet_ntoa(tmpAddr);

    // libconfig doesn't allow '.' or names to start with a number - so we have 
    // to hack up the address to match the "node_xx_xx_xx_xx" format, which is lame.
    for (unsigned int i=0; i<idAsAddr.length();i++)      
        if (idAsAddr[i]=='.')                            
            idAsAddr[i]='_';                             

    LOG_DEBUG("Looking up props for node under id: \"" << idAsAddr << "\""); 

    string prop("watcherProperties");
    Setting &root=cfg.getRoot();

    // libconfig doesn't like this 
    // nested data for some reason. Until I figure out why, don't use nested 
    // groups. 
    //     if (!root.exists(prop))
    //         root.add(prop, Setting::TypeGroup);
    //     else
    // 	LOG_DEBUG("Found existing watcherProperties in configuration"); 
    // 
    //     Setting &wp=cfg.lookup(prop); 
    // 
    //     prop=idAsAddr;
    //     if (!wp.exists(prop))
    //         wp.add(prop, Setting::TypeGroup);
    //     else
    //     	LOG_DEBUG("Did not find existing props for node \"" << prop << "\""); 
    
    //    Setting &nodeProps=wp[prop];

	if (!root.exists(idAsAddr)) 
	{
		LOG_DEBUG("Did not find node props in config file, creating new entry"); 
		root.add(idAsAddr, Setting::TypeGroup);
	}

    Setting &nodeProps=cfg.lookup(idAsAddr); 

    // ------- Label ----------------
    // Default label is last octets of the identifier
    string strVal;
    prop="label";
    if (nodeProps.lookupValue(prop, strVal))
    {
        snprintf(propp->guiLabel, sizeof(propp->guiLabel), "%s", strVal.c_str());
        LOG_DEBUG("Using existing label from cfg file: " << propp->guiLabel);
    }
    else
    {
        // snprintf(propp->guiLabel, sizeof(propp->guiLabel), "%s", defLabel.c_str()); 
	unsigned int tmp=ntohl(tmpAddr.s_addr); 	
        snprintf(propp->guiLabel, sizeof(propp->guiLabel), "%d.%d", (tmp >> 8 & 0xFF), (tmp & 0xFF));
        strVal=propp->guiLabel;

        LOG_DEBUG("label not found for this node, set to default: " << strVal);
        LOG_DEBUG("Storing property in config file: " << prop << " = " << strVal);
        nodeProps.add(prop, libconfig::Setting::TypeString)=strVal;
    }

    // ------- Shape ----------------
    prop="shape";
    int intVal=WATCHER_SHAPE_CIRCLE;
    if (nodeProps.lookupValue(prop, intVal))
    {
        LOG_DEBUG("Setting shape from cfg file to: " << intVal); 
        propp->shape=static_cast<WatcherShape>(intVal);
    }
    else
        nodeProps.add(prop, Setting::TypeInt)=intVal;

    // ------- Spin ----------------
    prop="spin";
    bool boolVal=false;
    if (nodeProps.lookupValue(prop, boolVal))
        propp->spin=boolVal;
    else
        nodeProps.add(prop, Setting::TypeBoolean)=boolVal;

    // ------- Flash ----------------
    prop="flash";
    boolVal=false;
    if (nodeProps.lookupValue(prop, boolVal))
        propp->flash=boolVal;
    else
        nodeProps.add(prop, Setting::TypeBoolean)=boolVal;

    // ------- Size ----------------
    prop="size";
    float floatVal=1.0;
    if (nodeProps.lookupValue(prop, floatVal))
        propp->size=floatVal;
    else
        nodeProps.add(prop, Setting::TypeFloat)=floatVal;

    sc.unlock();
    TRACE_EXIT();
}
