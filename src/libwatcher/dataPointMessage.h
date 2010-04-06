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
