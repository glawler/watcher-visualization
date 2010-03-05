#include "AlertHandlerDefault.h"
#include "XPathExtractor.h"
#include <string.h>
#include <libwatcher/labelMessage.h>
#include <libwatcher/colors.h>

using namespace HierarchyAPI;
using SpartaIssoSrdXmlAPI::XPathExtractor;
using namespace std; 
using namespace watcher;
using namespace watcher::event;

INIT_LOGGER(AlertHandlerDefault, "AlertHandler.AlertHandlerDefault"); 

AlertHandlerDefault::AlertHandlerDefault()
{
	// noop
}

// virtual
AlertHandlerDefault::~AlertHandlerDefault()
{
	// noop
}

//
// Default "handling" is too assume one victim and one attacker and 
// label thoses nodes in the watcher. 
//
bool AlertHandlerDefault::handleAlert(const xmlDoc* alertMessage, watcher::ClientPtr client)
{
    TRACE_ENTER(); 

    string* detectorAddrStr = XPathExtractor::extractFromDocUsing(alertMessage, 
            "/IDMEF-Message/Alert/Analyzer/Node/Address[@category='ipv4-addr']/address");
    string* attackerAddrStr = XPathExtractor::extractFromDocUsing(alertMessage, 
            "/IDMEF-Message/Alert/Source/Node/Address[@category='ipv4-addr']/address");
    string* victimAddrStr = XPathExtractor::extractFromDocUsing(alertMessage, 
            "/IDMEF-Message/Alert/Target/Node/Address[@category='ipv4-addr']/address");

    std::vector<MessagePtr> messages;

    if (attackerAddrStr) {
        string text("Attacking "); 
        if (victimAddrStr)
            text+=*victimAddrStr;
        LabelMessagePtr lm(new LabelMessage(text, NodeIdentifier::from_string(*attackerAddrStr)));
        lm->layer="Attackers";
        lm->background=watcher::colors::black;
        lm->foreground=watcher::colors::red;
        lm->expiration=m_alertLabelTimeout*1000;
        messages.push_back(lm); 
    }
    if (victimAddrStr) {
        string text("Victim "); 
        if (attackerAddrStr) {
            text+="of ";
            text+=*attackerAddrStr;
        }
        LabelMessagePtr lm(new LabelMessage(text, NodeIdentifier::from_string(*victimAddrStr)));
        lm->layer="Victims";
        lm->background=watcher::colors::black;
        lm->foreground=watcher::colors::red;
        lm->expiration=m_alertLabelTimeout*1000;
        messages.push_back(lm); 
    }
    if(detectorAddrStr) {
        string *classificationString = XPathExtractor::extractFromDocUsing(alertMessage, "/IDMEF-Message/Alert/Classification/@text");
        if(!classificationString->empty())
        {
            LabelMessagePtr lm(new LabelMessage(*classificationString, NodeIdentifier::from_string(*detectorAddrStr)));
            lm->layer="Detectors";
            lm->background=watcher::colors::black;
            lm->foreground=watcher::colors::white;
            lm->expiration=m_alertLabelTimeout*1000;
        }
        if(classificationString) 
            delete classificationString; 
    }

    if (!messages.empty()) 
        if (!client->sendMessages(messages))  
            LOG_WARN("Error sending alert messages to the watcher");

    if(victimAddrStr)   
        delete victimAddrStr; 
    if(attackerAddrStr) 
        delete attackerAddrStr; 
    if(detectorAddrStr) 
        delete detectorAddrStr; 

    TRACE_EXIT(); 
    return true; 
}
