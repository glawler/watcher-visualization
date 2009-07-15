/**
 * @file watcherRegion.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
#ifndef YOUDONTTOUCHTHEQUEENEVENIFYOUAREMICHELLEOBAMBA_H
#define YOUDONTTOUCHTHEQUEENEVENIFYOUAREMICHELLEOBAMBA_H

#include <ostream>
#include <boost/shared_ptr.hpp>
#include "logger.h"

namespace watcher
{
    /**
     * @class WatcherRegion. This class defines a region that exists in the watcher environment.
     * It is meant to be used as a filter to narrow the messages that are sent to an watcherd attached GUI.
     */
    class WatcherRegion
    {
        public: 
            /**
             * Contstructor
             */
            WatcherRegion();

            /**
             * Whatchoo talkin' 'bout Willis?
             */
            virtual ~WatcherRegion();

            /**
             * Write an instance of this class as a human readable stream to the otream given
             */
            virtual std::ostream &toStream(std::ostream &out) const;

            /**
             * Write an instance of this class as a human readable stream to the otream given.
             * Just calls toStream().
             */
            std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

        protected:

            // noop

        private:
            DECLARE_LOGGER();

    }; 

    /**
     * typedef a WatcherRegion shared pointer type
     */
    typedef boost::shared_ptr<WatcherRegion> WatcherRegionPtr;

    /** write a human readable version of the WatcherRegion class to the ostream given
    */
    std::ostream &operator<<(std::ostream &out, const WatcherRegion &region);
}

#endif //  YOUDONTTOUCHTHEQUEENEVENIFYOUAREMICHELLEOBAMBA_H
