#ifndef ALERT_HANDLERS_H_WERE_KNIGHTS_OF_THE_ROUND_TABLE_WE_DANCE_WHENEER_WERE_ABLE
#define ALERT_HANDLERS_H_WERE_KNIGHTS_OF_THE_ROUND_TABLE_WE_DANCE_WHENEER_WERE_ABLE

#include <libwatcher/client.h>
#include "AlertHandler.h"
#include "AlertHandlerDefault.h"
#include "logger.h"
#include <map>

namespace HierarchyAPI
{
	// This class is a singleton which holds all known 
	// types of alert handlers. 
	// It hides the alert types, etc, from the callers. 

	class AlertHandlers
	{
		public:
			// get a pointer to the one and only alertHanadlers instance. 
			// The pointer is not thread safe. 
			static AlertHandlers *getAlertHandlers(); 	

			//
			// react somehow if this alert is known
			// 
			bool handleAlert(const xmlDoc* alertMessage, watcher::ClientPtr client); 

			//
			// Set GUI alert timeout. (Breaks encapsulation)
			//
			void setGUIAlertTimeout(const int &timeout); 

		protected:
		
			AlertHandlers(); 	// singleton
			~AlertHandlers();	// singleton

		private:
            DECLARE_LOGGER();

			// no copies of singleton class. 
			AlertHandlers(const AlertHandlers &noCopiesThanks); 

			// map of classification string to handler. 
			std::map<std::string, AlertHandler *> m_alertHandlers; 

			// default handler if we don't know how to handle an alert. 
			AlertHandlerDefault m_defaultHandler; 
	};
}

#endif // ALERT_HANDLERS_H_WERE_KNIGHTS_OF_THE_ROUND_TABLE_WE_DANCE_WHENEER_WERE_ABLE
