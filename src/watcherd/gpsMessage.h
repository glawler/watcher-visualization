#ifndef WATCHER_GPS__MESSAGE_DATA_H
#define WATCHER_GPS__MESSAGE_DATA_H

#include <string>

#include "logger.h"
#include "message.h"

namespace boost {
    namespace archive {
        class polymorphic_iarchive;
        class polymorphic_oarchive;
    }
}

namespace watcher 
{
    class GPSMessage : public Message
    {
        public:
            // The data
            float lat;
            float lng;
            float alt;

            GPSMessage(const float &lat=0.0, const float &lng=0.0, const float &alt=0.0);
            GPSMessage(const GPSMessage &other);

            bool operator==(const GPSMessage &other) const;
            GPSMessage &operator=(const GPSMessage &other);

            virtual std::ostream &toStream(std::ostream &out) const;
            std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

            virtual void serialize(boost::archive::polymorphic_iarchive & ar, const unsigned int file_version);
            virtual void serialize(boost::archive::polymorphic_oarchive & ar, const unsigned int file_version);

        private:
            DECLARE_LOGGER();
    };

    typedef boost::shared_ptr<GPSMessage> GPSMessagePtr; 

    std::ostream &operator<<(std::ostream &out, const GPSMessage &mess);
}

#endif // WATCHER_GPS__MESSAGE_DATA_H
