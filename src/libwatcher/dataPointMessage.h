/**
 * @file dataPointMessage.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
#ifndef WATCHER_DATA_POINT_MESSAGE
#define WATCHER_DATA_POINT_MESSAGE

#include <string>
#include "message.h"

namespace watcher {
    namespace event {
        /** 
         * @class DataPointMessage
         *
         * Command line executable for this message: @ref sendDataPointMessage
         */
        class DataPointMessage : public Message {
            public:

                // The data
                //
                // Does data have a layer? Should it?
                // GUILayer layer; 

                /** What type of data this is. "Bandwidth", "Uptime", etc. */
                std::string dataName;

                /** The data itself for this period */
                typedef std::vector<double> DataPointList;
                DataPointList dataPoints;

                /** Let there be default light */
                DataPointMessage(); 

                /** Let there be a specific light */
                DataPointMessage(const std::string &dataName, const DataPointList &data);

                /** clone me */
                DataPointMessage(const DataPointMessage &other);

                /** judge me */
                bool operator==(const DataPointMessage &other) const;

                /** make me equal */
                DataPointMessage &operator=(const DataPointMessage &other);

                /** Drop me in the water. */
                virtual std::ostream &toStream(std::ostream &out) const;
                std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

            private:
                friend class boost::serialization::access;
                template <typename Archive> void serialize(Archive& ar, const unsigned int file_version);
                DECLARE_LOGGER();
        };

        typedef boost::shared_ptr<DataPointMessage> DataPointMessagePtr; 

        std::ostream &operator<<(std::ostream &out, const DataPointMessage &mess);
    }
}
#endif // WATCHER_DATA_POINT_MESSAGE
