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
 * @file edgeMessage.h
 * @author Geoff Lawler <geoff.lawer@cobham.com>
 * @date 2009-07-15
 */
#include <iosfwd>

#include "marshalYAML.h"
#include "edgeMessage.h"
#include "messageTypesAndVersions.h"
#include "watcherGlobalFunctions.h"         // for address serialize(). 
#include "colors.h"
#include "logger.h"

using namespace std;
using namespace boost;


namespace watcher {
    namespace event {

        INIT_LOGGER(EdgeMessage, "Message.EdgeMessage");

        EdgeMessage::EdgeMessage(
                                 const NodeIdentifier &node1_,        
                                 const NodeIdentifier &node2_,        
                                 const GUILayer &layer_,                 
                                 const Color &c_,
                                 const float &width_,
                                 const bool bidirectional_,
                                 Timestamp expiration_, 
                                 const bool &addEdge_) : 
            Message(EDGE_MESSAGE_TYPE, EDGE_MESSAGE_VERSION),
            node1(node1_),
            node2(node2_),
            edgeColor(c_),
            expiration(expiration_),
            width(width_),
            layer(layer_),
            addEdge(addEdge_),
            middleLabel(),
            node1Label(),
            node2Label(),
            bidirectional(bidirectional_)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        EdgeMessage::EdgeMessage() : 
            Message(EDGE_MESSAGE_TYPE, EDGE_MESSAGE_VERSION),
            node1(),
            node2(),
            edgeColor(colors::blue),
            expiration(Infinity), 
            width(2.0),
            layer(PHYSICAL_LAYER),
            addEdge(true),
            middleLabel(),
            node1Label(),
            node2Label(),
            bidirectional(false)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        EdgeMessage::EdgeMessage(const EdgeMessage &other) :
            Message(other.type, other.version),
            node1(other.node1),
            node2(other.node2),
            edgeColor(other.edgeColor),
            expiration(other.expiration), 
            width(other.width),
            layer(other.layer),
            addEdge(other.addEdge),
            middleLabel(other.middleLabel),
            node1Label(other.node1Label),
            node2Label(other.node2Label),
            bidirectional(other.bidirectional)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        void EdgeMessage::setMiddleLabel(const LabelMessagePtr &label)
        {
            TRACE_ENTER();
            middleLabel=label;
            TRACE_EXIT();
        }
        void EdgeMessage::setNode1Label(const LabelMessagePtr &label) 
        {
            TRACE_ENTER();
            node1Label=label;
            node1Label->fromNodeID=node1;
            TRACE_EXIT();
        }
        void EdgeMessage::setNode2Label(const LabelMessagePtr &label)
        {
            TRACE_ENTER();
            node2Label=label;
            node2Label->fromNodeID=node2;
            TRACE_EXIT();
        }

        bool EdgeMessage::operator==(const EdgeMessage &other) const
        {
            TRACE_ENTER();

            bool retVal = 
                Message::operator==(other) && 
                node1==other.node1 && 
                node2==other.node2 && 
                bidirectional==other.bidirectional &&
                layer==other.layer;

            // Other member variables are not distinguishing features

            TRACE_EXIT_RET(retVal);
            return retVal;
        }

        EdgeMessage &EdgeMessage::operator=(const EdgeMessage &other)
        {
            TRACE_ENTER();

            Message::operator=(other);
            node1=other.node1;
            node2=other.node2;
            edgeColor=other.edgeColor;
            expiration=other.expiration;
            width=other.width;
            layer=other.layer;
            addEdge=other.addEdge;
            middleLabel=other.middleLabel;
            node1Label=other.node1Label;
            node2Label=other.node2Label;
            bidirectional=other.bidirectional;

            TRACE_EXIT();
            return *this;
        }

        // virtual 
        std::ostream &EdgeMessage::toStream(std::ostream &out) const
        {
            TRACE_ENTER();

            Message::toStream(out);
            out << " node1: " << node1;
            out << " node2: " << node2;
            out << " edgeColor: " << edgeColor;
            out << " expiration: " << expiration;
            out << " width: " << width;
            out << " dir: " << (bidirectional ? "bidirectional" : "unidirectional");
            out << " layer: " << layer;
            out << " add: " << (addEdge ? "true" : "false"); 

            // shared_ptr doesn't have ?: overloaded
            out << " node1Label: ";
            if (node1Label) out << *node1Label;
            else out << " NULL "; 

            out << " middleLabel: ";
            if (middleLabel) out << *middleLabel;
            else out << " NULL "; 

            out << " node2Label: ";
            if (node2Label) out << *node2Label;
            else out << " NULL "; 

            TRACE_EXIT();
            return out;
        }

        ostream& operator<<(ostream &out, const EdgeMessage &mess)
        {
            mess.operator<<(out);
            return out;
        }

		YAML::Emitter &EdgeMessage::serialize(YAML::Emitter &e) const {
			e << YAML::Flow << YAML::BeginMap;
			Message::serialize(e); 
			e << YAML::Key << "node1" << YAML::Value << node1.to_string(); 
			e << YAML::Key << "node2" << YAML::Value <<  node2.to_string(); 
			e << YAML::Key << "edgeColor" << YAML::Value << edgeColor.toString(); 
			e << YAML::Key << "expiration" << YAML::Value << expiration;
			e << YAML::Key << "width" << YAML::Value << width;
			e << YAML::Key << "layer" << YAML::Value << layer;
			e << YAML::Key << "addEdge" << YAML::Value << addEdge;
			if (node1Label) {
				e << YAML::Key << "node1Label" << YAML::Value; 
				node1Label->serialize(e); 
			}
			if (middleLabel) { 
				e << YAML::Key << "middleLabel" << YAML::Value; 
				middleLabel->serialize(e); 
			}
			if (node2Label) {
				e << YAML::Key << "node2Label" << YAML::Value; 
				node2Label->serialize(e); 
			}
			e << YAML::Key << "bidirectional" << YAML::Value << bidirectional;
			e << YAML::EndMap; 
			return e; 
		}
		YAML::Node &EdgeMessage::serialize(YAML::Node &node) {
			// Do not serialize base data GTL - Message::serialize(node); 
			string str; 
			node["node1"] >> str; 
			node1=NodeIdentifier::from_string(str); 
			node["node2"] >> str; 
			node2=NodeIdentifier::from_string(str); 
			node["edgeColor"] >> str; 
			edgeColor.fromString(str); 
			node["expiration"] >> expiration;
			node["width"] >> width;
			node["layer"] >> layer;
			node["addEdge"] >> addEdge;
			const YAML::Node *subNode;
			if (NULL!=(subNode=node.FindValue("node1Label"))) {
				node1Label=LabelMessagePtr(new LabelMessage); 
				node1Label->serialize(*(const_cast<YAML::Node*>(subNode))); 
			}
			if (NULL!=(subNode=node.FindValue("middleLabel"))) {
				middleLabel=LabelMessagePtr(new LabelMessage);
				middleLabel->serialize(*(const_cast<YAML::Node*>(subNode))); 
			}
			if (NULL!=(subNode=node.FindValue("node2Label"))) {
				node2Label=LabelMessagePtr(new LabelMessage);
				node2Label->serialize(*(const_cast<YAML::Node*>(subNode))); 
			}
			node["bidirectional"] >> bidirectional;
			return node;
		}
    }
}

