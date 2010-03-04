#ifndef ALERT_HANDLER_DEFAULT_H_WERE_OPERA_MAD_IN_CAMELOT_WE_SING_FROM_THE_DIAPHRAGM_A_LOT
#define ALERT_HANDLER_DEFAULT_H_WERE_OPERA_MAD_IN_CAMELOT_WE_SING_FROM_THE_DIAPHRAGM_A_LOT

#include <libwatcher/client.h>
#include "AlertHandler.h"
#include "logger.h"

namespace  HierarchyAPI
{
	class AlertHandlerDefault : public AlertHandler 
	{
		public:
			AlertHandlerDefault();
			virtual ~AlertHandlerDefault(); 

			bool handleAlert(const xmlDoc* alertMessage, watcher::ClientPtr client); 
		
		protected:

		
		private:
            DECLARE_LOGGER();

			// no copying or assignment
			AlertHandlerDefault& operator=(const AlertHandlerDefault &other); 
			AlertHandlerDefault(const AlertHandlerDefault &noCopies); 
	}; 

}

#endif // ALERT_HANDLER_DEFAULT_H_WERE_OPERA_MAD_IN_CAMELOT_WE_SING_FROM_THE_DIAPHRAGM_A_LOT
