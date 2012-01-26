/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Affero General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Affero General Public License for more details.
 * 
 *     You should have received a copy of the GNU Affero General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file gpsMessage.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
#ifndef WATCHER_GPS__MESSAGE_DATA_H
#define WATCHER_GPS__MESSAGE_DATA_H

#include <string>
#include <yaml.h>

#include "message.h"
#include "messageTypesAndVersions.h"

namespace watcher {
    namespace event {
        /** 
         * @class GPSMessage
         * This class encapsulates a message containing current coordinates for
         * a node. This message is sent to a watcherd instance. The GUI(s)
         * attached to that watcherd instance then use this information to
         * update the location of the node that this message references. 
         *
         * This class supports lat/long/alt and UTM coordinates. 
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
                 * @param x the x coordinate
                 * @param y the y coordinate
                 * @param z the z coordinate
                 */
                GPSMessage(const double &x=0.0, const double &y=0.0, const double &z=0.0);

                /** Copy a GPSMessage
                 * @param other the message to copy 
                 */
                GPSMessage(const GPSMessage &other);

                /** Compare two GPSMessage */
                bool operator==(const GPSMessage &other) const;

                /** Set a GPSMessage equal to another 
                 * @param other the GPSMessage to set this instance to be equal to
                 */
                GPSMessage &operator=(const GPSMessage &other);

                /** Write the GPS message to <b>out</b> in human readable format 
                 * @param out the stream to write to
                 * @return the stream that was written to
                 */
                virtual std::ostream &toStream(std::ostream &out) const;
                /** Write the GPS message to <b>out</b> in human readable format 
                 * @param out the stream to write to
                 * @return the stream that was written to
                 */
                std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

				/** Serialize this message using a YAML::Emitter
				 * @param e the emitter to serialize to
				 * @return the emitter emitted to.
				 */
				virtual YAML::Emitter &serialize(YAML::Emitter &e) const; 

				/** Serialize from a YAML::Node. 
				 * @param p the Node to read from 
				 * @return the parser read from. 
				 */
				virtual YAML::Node &serialize(YAML::Node &node); 

            private:
                DECLARE_LOGGER();
        };

        typedef boost::shared_ptr<GPSMessage> GPSMessagePtr; 

        std::ostream &operator<<(std::ostream &out, const GPSMessage &mess);
    }
}

#endif // WATCHER_GPS__MESSAGE_DATA_H
