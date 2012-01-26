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
 * @file dataPointMessage.cpp
 * @author Geoff Lawler <geoff.lawer@cobham.com>
 * @date 2009-07-15
 */
#include <boost/foreach.hpp>

#include "dataPointMessage.h"
#include "logger.h"

using namespace std;
using namespace boost;

namespace watcher {
    namespace event {
        INIT_LOGGER(DataPointMessage, "Message.DataPointMessage");

        DataPointMessage::DataPointMessage() : 
            Message(DATA_POINT_MESSAGE_TYPE, DATA_POINT_MESSAGE_VERSION),
            dataName(),
            dataPoints()
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }


        DataPointMessage::DataPointMessage(const std::string &dn, const DataPointList &data) : 
            Message(COLOR_MESSAGE_TYPE, COLOR_MESSAGE_VERSION),
            dataName(dn), 
            dataPoints(data)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        DataPointMessage::DataPointMessage(const DataPointMessage &other) : 
            Message(other),
            dataName(other.dataName), 
            dataPoints(other.dataPoints)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        bool DataPointMessage::operator==(const DataPointMessage &other) const
        {
            TRACE_ENTER();

            bool retVal = 
                Message::operator==(other) && 
                dataName==other.dataName && 
                dataPoints==other.dataPoints;

            TRACE_EXIT_RET(retVal);
            return retVal;
        }

        DataPointMessage &DataPointMessage::operator=(const DataPointMessage &other)
        {
            TRACE_ENTER();

            Message::operator=(other);
            dataName=other.dataName;
            dataPoints=other.dataPoints;

            TRACE_EXIT();
            return *this;
        }

        // virtual 
        std::ostream &DataPointMessage::toStream(std::ostream &out) const
        {
            TRACE_ENTER();

            Message::toStream(out);
            out << " name: " << dataName; 
            out << " data: ["; 
            BOOST_FOREACH(double point, dataPoints)
                out << " " << point;
            out << "]"; 

            TRACE_EXIT();
            return out;
        }

        ostream& operator<<(ostream &out, const DataPointMessage &mess)
        {
            mess.operator<<(out);
            return out;
        }

		YAML::Emitter &DataPointMessage::serialize(YAML::Emitter &e) const {
			e << YAML::Flow << YAML::BeginMap;
			Message::serialize(e); 
			e << YAML::Key << "dataName" << YAML::Value << dataName; 
			// yaml-cpp groks STL
			e << YAML::Key << "dataPoints" << YAML::Value << dataPoints;	
			e << YAML::EndMap; 
			return e; 
		}
		YAML::Node &DataPointMessage::serialize(YAML::Node &node) {
			// Do not serialize base data GTL - Message::serialize(node); 
			node["dataName"] >> dataName; 
			node["dataPoints"] >> dataPoints; 
			return node;
		}
    }
}
