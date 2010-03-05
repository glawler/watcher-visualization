#include "AlertHandlerFictitiousNeighbor.h"

using namespace HierarchyAPI; 

INIT_LOGGER(AlertHandlerFictitiousNeighbor, "AlertHandler.AlertHandlerFictitiousNeighbor"); 

AlertHandlerFictitiousNeighbor::AlertHandlerFictitiousNeighbor()
{
	// noop
}

//virtual 
AlertHandlerFictitiousNeighbor::~AlertHandlerFictitiousNeighbor()
{
	// noop 
}

bool AlertHandlerFictitiousNeighbor::handleAlert(const xmlDoc* alertMessage, watcher::ClientPtr client)
{
	return false; // let the default handle it. 
}
