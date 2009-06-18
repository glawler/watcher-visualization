#ifndef WATCHER_COLOR__MESSAGE_DATA_H
#define WATCHER_COLOR__MESSAGE_DATA_H

#include <string>
#include <boost/asio.hpp>

#include "message.h"
#include "watcherColors.h"
#include "watcherTypes.h" 

namespace watcher {
    namespace event {
        class ColorMessage : public Message {
            public:
                // The data
                Color color;
                Timestamp flashPeriod; 
                Timestamp expiration;  

                GUILayer layer; 

                ColorMessage();  // default: black, 127.0.0.1, no expire, no flash
                ColorMessage(
                             const Color &c, 
                             const boost::asio::ip::address &address=boost::asio::ip::address::from_string("127.0.0.1"),
                             const Timestamp &expiration=0,
                             const Timestamp &flashPeriod=0);
                ColorMessage(const ColorMessage &other);

                bool operator==(const ColorMessage &other) const;
                ColorMessage &operator=(const ColorMessage &other);

                virtual std::ostream &toStream(std::ostream &out) const;
                std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

            private:
                friend class boost::serialization::access;
                template <typename Archive> void serialize(Archive& ar, const unsigned int file_version);
                DECLARE_LOGGER();
        };

        typedef boost::shared_ptr<ColorMessage> ColorMessagePtr; 

        std::ostream &operator<<(std::ostream &out, const ColorMessage &mess);
    }
}
#endif // WATCHER_COLOR__MESSAGE_DATA_H
