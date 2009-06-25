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
        class PlaybackTimeRange : public Message {
            public:
                PlaybackTimeRange(); 

                Timestamp min_;
                Timestamp max_;
            private:
                template <typename Archive> void serialize(Archive & ar, const unsigned int version);
                friend class boost::serialization::access;
                DECLARE_LOGGER();
        };

        std::ostream& operator<< (std::ostream&, const PlaybackTimeRange&);

        typedef boost::shared_ptr<PlaybackTimeRange> PlaybackTimeRangePtr;

        bool operator== (const PlaybackTimeRange& lhs, const PlaybackTimeRange& rhs);
    }
}
#endif
