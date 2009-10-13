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
 * @file gpsMessage.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
#include "watcherSerialize.h"
#include "gpsMessage.h"
#include "logger.h"

using namespace std;

namespace watcher {
    namespace event {
        INIT_LOGGER(GPSMessage, "Message.GPSMessage");

        GPSMessage::GPSMessage(const double &x_, const double &y_, const double &z_) : 
            Message(GPS_MESSAGE_TYPE, GPS_MESSAGE_VERSION),
            x(x_),
            y(y_),
            z(z_),
            dataFormat(LAT_LONG_ALT_WGS84),
            layer(PHYSICAL_LAYER)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        GPSMessage::GPSMessage(const GPSMessage &other) :
            Message(other), 
            x(other.x),
            y(other.y),
            z(other.z),
            dataFormat(other.dataFormat),
            layer(other.layer)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        bool GPSMessage::operator==(const GPSMessage &other) const
        {
            TRACE_ENTER();

            bool retVal = 
                Message::operator==(other) && 
                x == other.x &&
                y == other.y &&
                z == other.z && 
                dataFormat == other.dataFormat &&
                layer == other.layer;

            TRACE_EXIT_RET(retVal);
            return retVal;
        }

        GPSMessage &GPSMessage::operator=(const GPSMessage &other)
        {
            TRACE_ENTER();

            Message::operator=(other);
            x = other.x;
            y = other.y;
            z = other.z;
            dataFormat = other.dataFormat;
            layer = other.layer; 

            TRACE_EXIT();
            return *this;
        }

        // virtual 
        std::ostream &GPSMessage::toStream(std::ostream &out) const
        {
            TRACE_ENTER();

            out << std::setprecision(std::numeric_limits<double>::digits10 + 2); 
            Message::toStream(out);
            out << " x: " << x;
            out << " y: " << y;
            out << " z: " << z;
            out << " format: " << dataFormat;
            out << " layer: " << layer; 

            TRACE_EXIT();
            return out;
        }

        ostream &operator<<(ostream &out, const GPSMessage &mess)
        {
            mess.operator<<(out);
            return out;
        }

        template <typename Archive> void GPSMessage::serialize(Archive & ar, const unsigned int /* file_version */)
        {
            TRACE_ENTER();
            ar & boost::serialization::base_object<Message>(*this);
            ar & x;
            ar & y;
            ar & z;
            ar & dataFormat;
            ar & layer; 
            TRACE_EXIT();
        }
    } // ns event
} // ns watcher

BOOST_CLASS_EXPORT(watcher::event::GPSMessage);
