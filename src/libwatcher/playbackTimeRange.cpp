/** @file
 * @author Michael Elkins <michael.elkins@cobham.com>
 * @date 2009-06-24
 */
#include "watcherSerialize.h"
#include "playbackTimeRange.h"

namespace watcher {
    namespace event {
        INIT_LOGGER(PlaybackTimeRangeMessage, "Message.PlaybackTimeRangeMessage");

        PlaybackTimeRangeMessage::PlaybackTimeRangeMessage()
            : Message(PLAYBACK_TIME_RANGE_MESSAGE_TYPE, PLAYBACK_TIME_RANGE_MESSAGE_VERSION),
            min_(0), max_(0)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        std::ostream& operator<< (std::ostream& os, const PlaybackTimeRangeMessage& m)
        {
            return os << "PlaybackTimeRangeMessage(" << m.min_ << ',' << m.max_ << ')';
        }

        bool operator== (const PlaybackTimeRangeMessage& lhs , const PlaybackTimeRangeMessage& rhs) {
            return lhs.min_ == rhs.min_ && lhs.max_ == rhs.max_;
        };

        template <typename Archive> void PlaybackTimeRangeMessage::serialize(Archive & ar, const unsigned int /* version */)
        {
            TRACE_ENTER();
            ar & boost::serialization::base_object<Message>(*this);
            ar & min_;
            ar & max_;
            TRACE_EXIT();
        }

    }
}

BOOST_CLASS_EXPORT(watcher::event::PlaybackTimeRangeMessage);
