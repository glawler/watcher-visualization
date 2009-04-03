#ifndef THIS_ONE_TIME_AT_BAND_CAMP____H
#define THIS_ONE_TIME_AT_BAND_CAMP____H

#include <ostream>
#include <string>
#include <boost/shared_ptr.hpp>
#include "libwatcher/watcherRegion.h"
#include "logger.h"

namespace watcher
{
    /** 
     * @class MessageStreamFilter
     * @author Geoff Lawler <geoff.lawler@sparta.com>
     * @date 2009-04-03
     */
    class MessageStreamFilter
    {
        public:
            /**
             * MessageStreamFilter
             * Create a filter which filters a message stream.
             */
            MessageStreamFilter();

            /**
             * Death to all humans!
             */
            virtual ~MessageStreamFilter();

            /**
             * getLayer()
             * @return Returns the current layer of this filter
             */
            std::string getLayer() const; 

            /**
             * setLayer()
             * @param layer - set the layer of this filter to be the value passed in.
             */
            void setLayer(const std::string &layer); 

            /**
             * getMessageType()
             * @return Returns the current message type of this filter
             */
            float getMessageType() const; 

            /**
             * setMessageType()
             * @param type - set the messageType of this filter to be the value passed in.
             */
            void setMessageType(const float &type); 

            /**
             * getRegion()
             * @return Returns the current region of this filter
             */
            std::string getRegion() const; 

            /**
             * setRegion()
             * @param region - set the region of this filter to be the value passed in.
             */
            void setRegion(const WatcherRegion &layer); 

            /**
             * Write an instance of this class as a human readable stream to the otream given
             */
            virtual std::ostream &toStream(std::ostream &out) const;

            /**
             * Write an instance of this class as a human readable stream to the otream given.
             * Just calls MessageStream::toStream().
             */
            std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

        protected:

            // noop

        private:
            DECLARE_LOGGER();

            std::string layer;
            unsigned int messageType;  
            WatcherRegion region;

    }; // like a graduating senior

    /**
     * @typedef a MessageStream shared pointer type
     */
    typedef boost::shared_ptr<MessageStreamFilter> MessageStreamFilterPtr;

    /** write a human readable version of the MessageStreamFilter class to the ostream given
     */
    std::ostream &operator<<(std::ostream &out, const MessageStreamFilter &messStreamFilter);

}

#endif //  THIS_ONE_TIME_AT_BAND_CAMP____H
