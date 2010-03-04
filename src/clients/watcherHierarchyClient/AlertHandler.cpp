#include "AlertHandler.h"
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace HierarchyAPI;
using namespace std; 

INIT_LOGGER(AlertHandler, "AlertHandler"); 

AlertHandler::AlertHandler() : m_alertLabelTimeout(12)
{
	// noop
}

// virtual
AlertHandler::~AlertHandler()
{
	// noop
}

// declared private
AlertHandler::AlertHandler(const AlertHandler &noCopies)
{

}

void AlertHandler::setGUIAlertTimeout(const int &timeout)
{
	m_alertLabelTimeout = timeout; 
	LOG_INFO("New GUI alert timeout is " << timeout); 
}

