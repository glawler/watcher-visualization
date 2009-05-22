#ifndef WATCHER_GPS__MESSAGE_DATA_H
#define WATCHER_GPS__MESSAGE_DATA_H

#include <string>

#include "message.h"

namespace watcher {
    namespace event {
        class GPSMessage : public Message {
            public:

                // Supported GPS data formats. Default is LAT_LONG_ALT_WGS84.
                enum DataFormat
                {
                    LAT_LONG_ALT_WGS84,
                    UTM
                }; 

                // The data
                double x;
                double y;
                double z;

                DataFormat dataFormat;

                // Only valid if format==UTM
                std::string utmZoneReference;

                GPSMessage(const double &x=0.0, const double &y=0.0, const double &z=0.0);
                GPSMessage(const GPSMessage &other);

                bool operator==(const GPSMessage &other) const;
                GPSMessage &operator=(const GPSMessage &other);

                virtual std::ostream &toStream(std::ostream &out) const;
                std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

            private:
                friend class boost::serialization::access;
                template <typename Archive> void serialize(Archive & ar, const unsigned int file_version);
                DECLARE_LOGGER();
        };

        typedef boost::shared_ptr<GPSMessage> GPSMessagePtr; 

        std::ostream &operator<<(std::ostream &out, const GPSMessage &mess);
    }
}

#endif // WATCHER_GPS__MESSAGE_DATA_H
