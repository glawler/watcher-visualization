/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Affero General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Affero General Public License for more details.
 * 
 *     You should have received a copy of the GNU Affero General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file messageStreamFilter.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
#ifndef THIS_ONE_TIME_AT_BAND_CAMP____H
#define THIS_ONE_TIME_AT_BAND_CAMP____H

#include <ostream>
#include <boost/shared_ptr.hpp>
#include "watcherRegion.h"
#include "messageTypesAndVersions.h"  // for GUILayer
#include "message_fwd.h"
#include "declareLogger.h"

namespace watcher
{
    using namespace event; 
    
    /** 
      Create a filter which filters a message stream.
     * @author Geoff Lawler <geoff.lawler@sparta.com>
     * @date 2009-04-03
     */
    class MessageStreamFilter
    {
        public:
            /**
             * Create a filter which filters a message stream. If argument is true, then
             * all criteria in this filter will be ANDed when the filter is applied, 
             * otherwise they will be ORed. (AND==all criteria must match, OR==only one 
             * criteria must match). Default is OR.
             */
            MessageStreamFilter(bool opAND=false);

            /**
             * Clone me.
             */
            MessageStreamFilter(const MessageStreamFilter &copyme); 

            /**
             * Death to all humans!
             */
            virtual ~MessageStreamFilter();

            /** Does this message pass this filter?
             * Returns true if the message does *not* meet any 
             * of the criteria set in this filter instance.
             */
            bool passFilter(const MessagePtr m) const;

            /**
             * Returns the current layer of this filter
             */
            GUILayer getLayer() const; 

            /**
             * @param layer set the layer of this filter to be the value passed in.
             */
            void setLayer(const GUILayer &layer); 

            /**
             * @return Returns the current message type of this filter
             */
            unsigned int getMessageType() const; 

            /**
             * @param type set the messageType of this filter to be the value passed in.
             */
            void setMessageType(const unsigned int &type); 

            /**
             * @return the current region of this filter
             */
            WatcherRegion getRegion() const; 

            /**
             * @param region set the region of this filter to be the value passed in.
             */
            void setRegion(const WatcherRegion &region); 

            /** judge me */
            bool operator==(const MessageStreamFilter &other) const;

            /** make me equal */
            MessageStreamFilter &operator=(const MessageStreamFilter &other);

            /**
             * Write an instance of this class as a human readable stream to the otream given
             * @param out the output stream
             * @return reference to the output stream
             */
            virtual std::ostream &toStream(std::ostream &out) const;

            /**
             * Write an instance of this class as a human readable stream to the otream given.
             * Just calls MessageStream::toStream().
             * @param out the output stream
             * @return reference to the output stream
             */
            std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

            /** Annoying, but the easiest (only?) way to go when using boost::serialize
             * just pretented these are private for now. Blah.
             */
            std::string layer;
            unsigned int messageType;  
            WatcherRegion region;
            bool opAND;

        protected:

            // noop

        private:
            DECLARE_LOGGER();

    }; // like a graduating senior

    /// a MessageStream shared pointer type
    typedef boost::shared_ptr<MessageStreamFilter> MessageStreamFilterPtr;

    /** write a human readable version of the MessageStreamFilter class to the ostream given
     * @param out the output stream
     * @param messStreamFilter the filter to display
     * @return reference to the output stream
     */
    std::ostream &operator<<(std::ostream &out, const MessageStreamFilter &messStreamFilter);
}

#endif //  THIS_ONE_TIME_AT_BAND_CAMP____H
