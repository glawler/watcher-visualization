#include "AlertHandlerWormhole.h"
#include "XPathExtractor.h"
#include <string.h>
#include <libwatcher/labelMessage.h>
#include <libwatcher/edgeMessage.h>

using namespace HierarchyAPI; 
using namespace SpartaIssoSrdXmlAPI; 
using namespace watcher;
using namespace watcher::event;

INIT_LOGGER(AlertHandlerWormhole, "AlertHandler.AlertHandlerWormhole"); 

AlertHandlerWormhole::AlertHandlerWormhole()
{
	// noop
}

//virtual 
AlertHandlerWormhole::~AlertHandlerWormhole()
{
	// noop 
}

bool AlertHandlerWormhole::handleAlert(const xmlDoc* alertMessage, watcher::ClientPtr client)
{
	TRACE_ENTER();

	string* attacker1AddrStr=XPathExtractor::extractFromDocUsing(alertMessage, "/IDMEF-Message/Alert/Source[1]/Node/Address[@category='ipv4-addr']/address");
	string* attacker2AddrStr=XPathExtractor::extractFromDocUsing(alertMessage, "/IDMEF-Message/Alert/Source[2]/Node/Address[@category='ipv4-addr']/address");

	
	if(attacker1AddrStr && attacker2AddrStr)
	{
	    LOG_DEBUG("Marking " << attacker1AddrStr << " and " <<  attacker2AddrStr << " as ends of the wormhole"); 

        const string layer("Alerts"); 
        LabelMessagePtr lm(new LabelMessage("Wormhole detected"));
        lm->layer=layer;

        EdgeMessagePtr em(new EdgeMessage(NodeIdentifier::from_string(*attacker1AddrStr), NodeIdentifier::from_string(*attacker2AddrStr), layer));
        em->expiration=m_alertLabelTimeout*1000;

        std::vector<event::MessagePtr> messages;
        messages.push_back(lm);
        messages.push_back(em);
        if (!client->sendMessages(messages)) 
            LOG_WARN("Error sending wormhole alert messages to the watcher"); 
	}

	if(attacker1AddrStr) 
        delete attacker1AddrStr; 
	if(attacker2AddrStr) 
        delete attacker2AddrStr; 

	TRACE_EXIT();
	return true; 
}

