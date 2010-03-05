#include "AlertHandlerOmittedNeighbor.h"

using namespace HierarchyAPI; 

INIT_LOGGER(AlertHandlerOmittedNeighbor, "AlertHandler.AlertHandlerOmittedNeighbor"); 

AlertHandlerOmittedNeighbor::AlertHandlerOmittedNeighbor()
{
	// noop
}

//virtual 
AlertHandlerOmittedNeighbor::~AlertHandlerOmittedNeighbor()
{
	// noop 
}

bool AlertHandlerOmittedNeighbor::handleAlert(const xmlDoc* alertMessage, watcher::ClientPtr client)
{
	return false; // let the default handle it. 
}
