#ifndef ALERT_HANDLER_FC_H_BUT_MANY_TIMES_WERE_GIVEN_RHYMES_THAT_ARENT_QUITE_UNSINGABLE
#define ALERT_HANDLER_FC_H_BUT_MANY_TIMES_WERE_GIVEN_RHYMES_THAT_ARENT_QUITE_UNSINGABLE

#include <libwatcher/client.h>
#include "AlertHandler.h"
#include "logger.h"

namespace  HierarchyAPI
{
	class AlertHandlerFictitiousNeighbor : public AlertHandler 
	{
		public:
			AlertHandlerFictitiousNeighbor();
			virtual ~AlertHandlerFictitiousNeighbor(); 

			bool handleAlert(const xmlDoc* alertMessage, watcher::ClientPtr client); 
		
		protected:

		
		private:
            DECLARE_LOGGER();
			// no copies or assignment
			AlertHandlerFictitiousNeighbor(const AlertHandlerFictitiousNeighbor &noCopies); 
			AlertHandlerFictitiousNeighbor& operator=(const AlertHandlerFictitiousNeighbor &other); 
	}; 

}

#endif // ALERT_HANDLER_FC_H_BUT_MANY_TIMES_WERE_GIVEN_RHYMES_THAT_ARENT_QUITE_UNSINGABLE
