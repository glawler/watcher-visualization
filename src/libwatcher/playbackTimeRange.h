/**
 * @file playbackTimeRange.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
/** @file
 * @author Michael Elkins <michael.elkins@cobham.com>
 * @date 2009-06-24
 */
#ifndef PLAYBACK_TIME_RANGE_H
#define PLAYBACK_TIME_RANGE_H

#include "message.h"

namespace watcher {
    namespace event {

        /**
         * Get time range of events in the playback database
         * @author Michael Elkins <michael.elkins@cobham.com>
         * @date 2009-06-24
         */
        class PlaybackTimeRangeMessage : public Message {
            public:
                PlaybackTimeRangeMessage(); 

                Timestamp min_;
                Timestamp max_;
            private:
                template <typename Archive> void serialize(Archive & ar, const unsigned int version);
                friend class boost::serialization::access;
                DECLARE_LOGGER();
        };

        std::ostream& operator<< (std::ostream&, const PlaybackTimeRangeMessage&);

        typedef boost::shared_ptr<PlaybackTimeRangeMessage> PlaybackTimeRangeMessagePtr;

        bool operator== (const PlaybackTimeRangeMessage& lhs, const PlaybackTimeRangeMessage& rhs);
    }
}
#endif
