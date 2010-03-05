#ifndef ALERT_HANDLER_WORMHOLE_H_WE_DO_ROUTINES_AND_CHORUS_SCENES_WIHT_FOOTWORK_IMPECCABLE
#define ALERT_HANDLER_WORMHOLE_H_WE_DO_ROUTINES_AND_CHORUS_SCENES_WIHT_FOOTWORK_IMPECCABLE

#include <libwatcher/client.h>
#include "AlertHandler.h"
#include "logger.h"

namespace  HierarchyAPI
{
	class AlertHandlerWormhole : public AlertHandler 
	{
		public:
			AlertHandlerWormhole();
			virtual ~AlertHandlerWormhole(); 

			bool handleAlert(const xmlDoc* alertMessage, watcher::ClientPtr client); 
		
		protected:

		
		private:
            DECLARE_LOGGER();
			// no copying or assignment
			AlertHandlerWormhole(const AlertHandlerWormhole &noCopies); 
			AlertHandlerWormhole& operator=(const AlertHandlerWormhole &other); 
	}; 

}

#endif // ALERT_HANDLER_WORMHOLE_H_WE_DO_ROUTINES_AND_CHORUS_SCENES_WIHT_FOOTWORK_IMPECCABLE
