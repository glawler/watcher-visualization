/** @file
 * @author Michael Elkins <michael.elkins@sparta.com>
 * @date 2009-03-20
 */
#ifndef SEEK_WATCHER_MESSAGE_H
#define SEEK_WATCHER_MESSAGE_H

#include "watcherMessage.h"

namespace watcher {
    namespace watchapi {

        /**
         * Seek to a particular point in time in the event stream
         * @author Michael Elkins <michael.elkins@sparta.com>
         * @date 2009-03-20
         */
        class SeekMessage : public WatcherMessage {
            public:
                /** Type representing how to interpet an offset */
                enum whence {
                    start,      //< offset is relative to start of stream
                    cur,        //< offset is relative to current position in stream
                    end         //< offset is relative to end of stream
                };
                SeekMessage(float offset, whence);

            private:
                friend class boost::serialization::access;
                template <typename Archive> void serialize(Archive& ar, const unsigned int version);
                DECLARE_LOGGER();
                float offset;   //< offset into stream in seconds
                whence rel;     //< specifies how offset is interpreted
        };

        template <typename Archive>
        void SeekMessage::serialize(Archive& ar, const unsigned int version)
        {
            TRACE_ENTER();
            ar & offset;
            ar & rel;
            TRACE_EXIT();
        }
    }
}
#endif
