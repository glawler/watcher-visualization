#ifndef ALERT_HANDLER_ON_H_IN_WAR_WERE_TOUGH_AND_ABLE_QUITE_INDEFATIGABLE
#define ALERT_HANDLER_ON_H_IN_WAR_WERE_TOUGH_AND_ABLE_QUITE_INDEFATIGABLE

#include <libwatcher/client.h>
#include "AlertHandler.h"
#include "logger.h"

namespace  HierarchyAPI
{
	class AlertHandlerOmittedNeighbor: public AlertHandler 
	{
		public:
			AlertHandlerOmittedNeighbor();
			virtual ~AlertHandlerOmittedNeighbor(); 

			bool handleAlert(const xmlDoc* alertMessage, watcher::ClientPtr client); 
		
		protected:

		
		private:
            DECLARE_LOGGER();
			// no copying or assignment
			AlertHandlerOmittedNeighbor(const AlertHandlerOmittedNeighbor &noCopies); 
			AlertHandlerOmittedNeighbor& operator=(const AlertHandlerOmittedNeighbor &other); 
	}; 

}

#endif // ALERT_HANDLER_ON_H_IN_WAR_WERE_TOUGH_AND_ABLE_QUITE_INDEFATIGABLE
