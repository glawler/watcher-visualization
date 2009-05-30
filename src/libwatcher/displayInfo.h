#ifndef DISPLAY_INFO_H
#define DISPLAY_INFO_H

#include <boost/utility.hpp>  // for nonconfigurable
#include "logger.h"

namespace watcher
{
    /** 
     * @class DisplayInfo
     * @author Geoff.Lawler <Geoff.Lawler@cobham.com> 
     *
     * Base class for keeping track of display information. Derived classes
     * will be read in their own display configuration and use it to keep 
     * track of the state of the display of the object. 
     *
     */
    class DisplayInfo : 
        public boost::noncopyable   // for now, may change later
    {
        public:
            DisplayInfo(const std::string &category); 
            virtual ~DisplayInfo(); 

        protected:

            std::string categoryName;

        private:

            DECLARE_LOGGER();
    };
}

#endif // DISPLAY_INFO_H
