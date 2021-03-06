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
 * @file connectivityMessage.cpp
 * @author geoff lawler <geoff.lawler@cobham.com>
 * @date 2009-05-06
 */
#include <boost/foreach.hpp>
#include <logger.h>
#include "marshalYAML.h"
#include "connectivityMessage.h"

using namespace std;

namespace watcher 
{
    namespace event 
    {
        INIT_LOGGER(ConnectivityMessage, "Message.ConnectivityMessage");

        ConnectivityMessage::ConnectivityMessage() : 
            Message(CONNECTIVITY_MESSAGE_TYPE, CONNECTIVITY_MESSAGE_VERSION), 
            neighbors(),
            layer(PHYSICAL_LAYER)

        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        // virtual
        ConnectivityMessage::~ConnectivityMessage()  
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }
        ConnectivityMessage::ConnectivityMessage(const ConnectivityMessage &other) :
            Message(other.type, other.version), 
            neighbors(other.neighbors),
            layer(PHYSICAL_LAYER)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        bool ConnectivityMessage::operator==(const ConnectivityMessage &other) const
        {
            TRACE_ENTER();

            bool retVal = 
                Message::operator==(other) && 
                neighbors == other.neighbors &&
                layer == other.layer;

            TRACE_EXIT_RET(retVal);
            return retVal;
        }

        ConnectivityMessage &ConnectivityMessage::operator=(const ConnectivityMessage &other)
        {
            TRACE_ENTER();

            Message::operator=(other);
            neighbors=other.neighbors;
            layer=other.layer;

            TRACE_EXIT();
            return *this;
        }

        // virtual 
        std::ostream &ConnectivityMessage::toStream(std::ostream &out) const
        {
            TRACE_ENTER();

            Message::toStream(out);
            out << " layer: " << layer;
            out << " neighbors:[";
            for (vector<NodeIdentifier>::const_iterator i=neighbors.begin(); i != neighbors.end(); ++i)
                out << *i << ",";
            out << "] ";

            TRACE_EXIT();
            return out;
        }

        ostream &operator<<(ostream &out, const ConnectivityMessage &mess)
        {
            mess.operator<<(out);
            return out;
        }

		// YAML::Emitter &operator<<(YAML::Emitter &out, const watcher::event::ConnectivityMessage::NeighborList &ns); 
		// void operator>>(const YAML::Node& in, watcher::ConnectivityMessage::NeighborList &ns); 

		YAML::Emitter &ConnectivityMessage::serialize(YAML::Emitter &e) const {
			e << YAML::Flow << YAML::BeginMap;
			Message::serialize(e); 
			e << YAML::Key << "layer" << YAML::Value << layer;		
			e << YAML::Key << "neighbors" << YAML::Value;		
				e << YAML::Flow << YAML::BeginSeq; 
				BOOST_FOREACH(const NodeIdentifier &n, neighbors) 
					e << n.to_string(); 
				e << YAML::EndSeq; 
			e << YAML::EndMap; 

			return e; 
		}
		YAML::Node &ConnectivityMessage::serialize(YAML::Node &node) {
			// Do not serialize base data GTL - Message::serialize(node); 
			node["layer"] >> layer; 
			const YAML::Node &nbrs=node["neighbors"]; 
			for (unsigned i=0;i<nbrs.size();i++) {
				string str;
				nbrs[i] >> str; 
				neighbors.push_back(NodeIdentifier::from_string(str)); 
			}
			return node;
		}
    }
}

