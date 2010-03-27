#include "AlertHandlerDefault.h"
#include "XPathExtractor.h"
#include <boost/exception/diagnostic_information.hpp>
#include <string.h>
#include <libwatcher/labelMessage.h>
#include <libwatcher/edgeMessage.h>
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

    try {
        string* detectorAddrStr = XPathExtractor::extractFromDocUsing(alertMessage, 
                "/IDMEF-Message/Alert/Analyzer/Node/Address[@category='ipv4-addr']/address");
        string* attackerAddrStr = XPathExtractor::extractFromDocUsing(alertMessage, 
                "/IDMEF-Message/Alert/Source/Node/Address[@category='ipv4-addr']/address");
        string* victimAddrStr = XPathExtractor::extractFromDocUsing(alertMessage, 
                "/IDMEF-Message/Alert/Target/Node/Address[@category='ipv4-addr']/address");
        string *classificationString = XPathExtractor::extractFromDocUsing(alertMessage, "/IDMEF-Message/Alert/Classification[@text]");

        LOG_DEBUG("detectorAddrStr: " << *detectorAddrStr); 
        LOG_DEBUG("attackerAddrStr: " << *attackerAddrStr); 
        LOG_DEBUG("victimAddrStr: " << *victimAddrStr); 
        LOG_DEBUG("classificationString: " << *classificationString); 

        std::vector<MessagePtr> messages;

        if (attackerAddrStr) {
            string text("Attacking "); 
            if (victimAddrStr)
                text+=*victimAddrStr;
            if (classificationString) 
                text+=string(" (" + *classificationString + ")"); 
            LabelMessagePtr lm(new LabelMessage(text, NodeIdentifier::from_string(*attackerAddrStr)));
            lm->layer="Attackers";
            lm->background=watcher::colors::black;
            lm->foreground=watcher::colors::red;
            lm->expiration=m_alertLabelTimeout*1000;
            messages.push_back(lm); 
            LOG_DEBUG("Added label message to outbound queue: " << *lm); 
        }
        if (victimAddrStr) {
            string text("Victim "); 
            if (attackerAddrStr) {
                text+="of ";
                text+=*attackerAddrStr;
            }
            if (classificationString) 
                text+=string(" (" + *classificationString + ")"); 
            LabelMessagePtr lm(new LabelMessage(text, NodeIdentifier::from_string(*victimAddrStr)));
            lm->layer="Victims";
            lm->background=watcher::colors::black;
            lm->foreground=watcher::colors::red;
            lm->expiration=m_alertLabelTimeout*1000;
            messages.push_back(lm); 
        }
        if (attackerAddrStr && victimAddrStr) { 
            EdgeMessagePtr em(new EdgeMessage(
                        NodeIdentifier::from_string(*attackerAddrStr), 
                        NodeIdentifier::from_string(*victimAddrStr), 
                        "Attackers")); 
            em->edgeColor=watcher::colors::red;
            em->expiration=m_alertLabelTimeout*1000;
            messages.push_back(em); 

            EdgeMessagePtr em2(new EdgeMessage(
                        NodeIdentifier::from_string(*victimAddrStr), 
                        NodeIdentifier::from_string(*attackerAddrStr), 
                        "Victims")); 
            em2->edgeColor=watcher::colors::blue;
            em2->expiration=m_alertLabelTimeout*1000;
            messages.push_back(em2); 
        }
        if(detectorAddrStr) {
            if(classificationString)
            {
                LabelMessagePtr lm(new LabelMessage(*classificationString, NodeIdentifier::from_string(*detectorAddrStr)));
                lm->layer="Detectors";
                lm->background=watcher::colors::black;
                lm->foreground=watcher::colors::white;
                lm->expiration=m_alertLabelTimeout*1000;
                delete classificationString; 
            }
        }

        if (!messages.empty()) 
            if (!client) 
                LOG_WARN("Unable to send alert display messages to the watcher because we have no connection to the watcherd");
            else
                if (!client->sendMessages(messages))  
                    LOG_WARN("Error sending alert messages to the watcher");
                else
                    LOG_DEBUG("Sent alert display messages to the watcher"); 
        else
            LOG_DEBUG("Alert does not contain enough information to generate any display information"); 

        if(victimAddrStr)   
            delete victimAddrStr; 
        if(attackerAddrStr) 
            delete attackerAddrStr; 
        if(detectorAddrStr) 
            delete detectorAddrStr; 

    }
    catch (boost::exception &e) {
        LOG_ERROR("Caught boost exception when sending watcher messasge(s): " << diagnostic_information(e));
    }
    catch (std::exception &e) { 
        LOG_ERROR("caught: " << e.what()); 
    }

    TRACE_EXIT(); 
    return true; 
}
