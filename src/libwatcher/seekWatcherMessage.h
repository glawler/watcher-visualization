/** @file
 * @author Michael Elkins <michael.elkins@sparta.com>
 * @date 2009-03-20
 */
#ifndef SEEK_WATCHER_MESSAGE_H
#define SEEK_WATCHER_MESSAGE_H

#include "message.h"

namespace watcher {
    namespace event {

        /**
         * Seek to a particular point in time in the event stream
         * @author Michael Elkins <michael.elkins@sparta.com>
         * @date 2009-03-20
         */
        class SeekMessage : public Message {
            public:
                /** Type representing how to interpet an offset */
                enum whence {
                    start,      //< offset is relative to start of stream
                    cur,        //< offset is relative to current position in stream
                    end         //< offset is relative to end of stream
                };

                float offset;   //< offset into stream in seconds
                whence rel;     //< specifies how offset is interpreted

                SeekMessage(float offset = 0, whence = start);
                inline bool operator== (const SeekMessage &rhs) const { return offset == rhs.offset && rel == rhs.rel; }

                friend std::ostream& operator<< (std::ostream& o, const SeekMessage& m);

            private:
                friend class boost::serialization::access;
                template <typename Archive> void serialize(Archive& ar, const unsigned int version);
                DECLARE_LOGGER();
        };

        typedef boost::shared_ptr<SeekMessage> SeekMessagePtr;
    }
}
#endif
