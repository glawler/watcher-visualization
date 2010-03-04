#ifndef ALERT_HANDLER_H_WE_EAT_WELL_HERE_IN_CAMELOT_WE_EAT_HAM_AND_JAM_AND_SPAMALOT
#define ALERT_HANDLER_H_WE_EAT_WELL_HERE_IN_CAMELOT_WE_EAT_HAM_AND_JAM_AND_SPAMALOT

#include <libwatcher/client.h>
#include <libxml/tree.h>
#include "idsCommunications.h"
#include <string>

#include "logger.h"

namespace  HierarchyAPI
{
	class AlertHandler 
	{
		public:
			AlertHandler();
			virtual ~AlertHandler(); 

			// do actual work
			virtual bool handleAlert(const xmlDoc* alertMessage, watcher::ClientPtr client) = 0; 
		
			void setGUIAlertTimeout(const int &timeout);

		protected:

			int m_alertLabelTimeout; 

		private:

            DECLARE_LOGGER();
			AlertHandler(const AlertHandler &noCopies); 
	}; 

}

#endif // ALERT_HANDLER_H_WE_EAT_WELL_HERE_IN_CAMELOT_WE_EAT_HAM_AND_JAM_AND_SPAMALOT
