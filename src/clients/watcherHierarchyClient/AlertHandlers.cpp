#include "AlertHandlers.h"
#include "alertClassifications.h" 	// all known alert types
#include "XPathExtractor.h"

// known handlers
#include "AlertHandlerWormhole.h"
#include "AlertHandlerFictitiousNeighbor.h"
#include "AlertHandlerOmittedNeighbor.h"

using namespace HierarchyAPI; 
using namespace SpartaIssoSrdXmlAPI; 
using namespace std; 
using namespace watcher;
using namespace watcher::event;

INIT_LOGGER(AlertHandlers, "AlertHandlers"); 

// static
AlertHandlers *AlertHandlers::getAlertHandlers()
{
	static AlertHandlers theOneAndOnlyAlertHandlersInstance;
	return &theOneAndOnlyAlertHandlersInstance; 
}

bool AlertHandlers::handleAlert(const xmlDoc* alertMessage, ClientPtr client)
{
	string *classificationString = XPathExtractor::extractFromDocUsing(alertMessage, "/IDMEF-Message/Alert/Classification/@text");
	if(!classificationString->empty()) {
		std::map<std::string, AlertHandler *>::iterator handler = m_alertHandlers.find(*classificationString); 
		if(handler != m_alertHandlers.end()) {
			if(handler->second->handleAlert(alertMessage, client)) {
				delete classificationString; 
				return true; 
			}
			// else let the default handler get it. 
		}
		delete classificationString; // annoying. 
	}

	return m_defaultHandler.handleAlert(alertMessage, client); 
}

AlertHandlers::AlertHandlers() : m_defaultHandler()
{
#if !defined(NO_DEFAULT_ALERT_HANDLERS) || !NO_DEFAULT_ALERT_HANDLERS
	m_alertHandlers[SIMPLE_WORMHOLE_DETECTION_CLASSIFICATION_REFERENCE_NAME_TEXT] = new AlertHandlerWormhole(); 
	m_alertHandlers[FICTITIOUS_NEIGHBOR_CLASSIFICATION_REFERENCE_NAME_TEXT] = new AlertHandlerFictitiousNeighbor();
	m_alertHandlers[OMITTED_NEIGHBOR_CLASSIFICATION_REFERENCE_NAME_TEXT] = new AlertHandlerOmittedNeighbor(); 
#endif
}

AlertHandlers::~AlertHandlers()
{
	std::map<std::string, AlertHandler *>::iterator i;
	for(i = m_alertHandlers.begin(); i != m_alertHandlers.end(); ++i) {
		delete i->second; 
		i->second=NULL;
	}
}

void AlertHandlers::setGUIAlertTimeout(const int &timeout)
{
	std::map<std::string, AlertHandler *>::iterator i;
	for(i = m_alertHandlers.begin(); i != m_alertHandlers.end(); ++i) {
		i->second->setGUIAlertTimeout(timeout); 
	}
}
