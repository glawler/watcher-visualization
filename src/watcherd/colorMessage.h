#ifndef WATCHER_COLOR__MESSAGE_DATA_H
#define WATCHER_COLOR__MESSAGE_DATA_H

#include <string>
#include <boost/asio.hpp>

#include "logger.h"
#include "message.h"
#include "watcherColors.h"

// Forward decls;
namespace boost {
    namespace archive {
        class polymorphic_iarchive;
        class polymorphic_oarchive;
    }
}

namespace watcher 
{
    class ColorMessage : public Message
    {
        public:
            // The data
            Color color;
            boost::asio::ip::address nodeAddr;
            uint32_t flashPeriod;
            uint32_t expiration;

            ColorMessage();  // default: black, 127.0.0.1, no expire, no flash
            ColorMessage(
                    const Color &c, 
                    const boost::asio::ip::address &address=boost::asio::ip::address::from_string("127.0.0.1"),
                    const uint32_t &expiration=0,
                    const uint32_t &flashPeriod=0);
            ColorMessage(const ColorMessage &other);

            bool operator==(const ColorMessage &other) const;
            ColorMessage &operator=(const ColorMessage &other);

            virtual std::ostream &toStream(std::ostream &out) const;
            std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

            virtual void serialize(boost::archive::polymorphic_iarchive & ar, const unsigned int file_version);
            virtual void serialize(boost::archive::polymorphic_oarchive & ar, const unsigned int file_version);

        private:
            DECLARE_LOGGER();
    };

    typedef boost::shared_ptr<ColorMessage> ColorMessagePtr; 

    std::ostream &operator<<(std::ostream &out, const ColorMessage &mess);
}

#endif // WATCHER_COLOR__MESSAGE_DATA_H
