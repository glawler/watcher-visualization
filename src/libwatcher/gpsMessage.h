/**
 * @file gpsMessage.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
#ifndef WATCHER_GPS__MESSAGE_DATA_H
#define WATCHER_GPS__MESSAGE_DATA_H

#include <string>

#include "message.h"
#include "messageTypesAndVersions.h"

namespace watcher {
    namespace event {
        /** 
         *
         * This class encapsulates a message containing current coordinates for a node. This message is sent to a watcherd instance. The GUI(s) 
         * attached to that watcherd instance then use this information to update the location of the node that this message references. 
         *
         * This class supports lat/long/alt and UTM coordinates. 
         *
         * @class GPSMessage
         *
         * Command line executable for this message: @ref sendGPSMessage
         */
        class GPSMessage : public Message {
            public:

                /**  
                 * @enum DataFormat
                 * The coordinate systems supported by the GPSMessage message.
                 * Default is LAT_LONG_ALT_WGS84.
                 */
                enum DataFormat
                {
                    LAT_LONG_ALT_WGS84,
                    UTM
                }; 

                /** current the x coordinate of the node */
                double x;

                /** current the y coordinate of the node */
                double y;

                /** current the z coordinate of the node */
                double z;

                /** The data format of the coordinate */
                DataFormat dataFormat;

                /** The UTM zone of the data (only valid if format==UTM) */
                std::string utmZoneReference;

                /** The layer that node is on */
                GUILayer layer; 

                /** Create a GPSMessage 
                 * @arg x, the x coordinate
                 * @arg y, the y coordinate
                 * @arg z, the z coordinate
                 */
                GPSMessage(const double &x=0.0, const double &y=0.0, const double &z=0.0);

                /** Copy a GPSMessage
                 * @arg other, the message to copy 
                 */
                GPSMessage(const GPSMessage &other);

                /** Compare two GPSMessage */
                bool operator==(const GPSMessage &other) const;

                /** Set a GPSMessage equal to another 
                 * @arg other, the GPSMessage to set this instance to be equal to
                 */
                GPSMessage &operator=(const GPSMessage &other);

                /** Write the GPS message to <b>out</b> in human readable format 
                 * @arg out, the stream to write to
                 * @return the stream that was written to
                 */
                virtual std::ostream &toStream(std::ostream &out) const;
                /** Write the GPS message to <b>out</b> in human readable format 
                 * @arg out, the stream to write to
                 * @return the stream that was written to
                 */
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
