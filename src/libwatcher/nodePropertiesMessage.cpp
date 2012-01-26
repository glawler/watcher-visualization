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
 * @file nodePropertiesMessage.cpp
 * @author Geoff Lawler <geoff.lawer@cobham.com>
 * @date 2009-09-04
 */
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>       // for iequals();

#include "marshalYAML.h"
#include "nodePropertiesMessage.h"
#include "messageTypesAndVersions.h"
#include "colors.h"
#include "logger.h"

using namespace std;
using namespace boost;

namespace watcher {
    namespace event {

        INIT_LOGGER(NodePropertiesMessage, "Message.NodePropertiesMessage");

        NodePropertiesMessage::NodePropertiesMessage()   :
            Message(NODE_PROPERTIES_MESSAGE_TYPE, NODE_PROPERTIES_MESSAGE_VERSION),
            layer(PHYSICAL_LAYER),
            color(colors::red),
            useColor(false),
            size(-1.0),
            shape(NOSHAPE),
            useShape(false),
            displayEffects(),
            nodeProperties()
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        NodePropertiesMessage::NodePropertiesMessage(const NodePropertiesMessage &other) : 
            Message(other),
            layer(other.layer),
            color(other.color),
            useColor(other.useColor),
            size(other.size),
            shape(other.shape),
            useShape(other.useShape),
            displayEffects(other.displayEffects),
            nodeProperties(other.nodeProperties)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        NodePropertiesMessage::~NodePropertiesMessage()
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        bool NodePropertiesMessage::operator==(const NodePropertiesMessage &other) const
        {
            TRACE_ENTER();

            bool retVal = 
                Message::operator==(other) && 
                layer==other.layer &&
                color==other.color &&
                useColor==other.useColor &&
                size==other.size &&
                shape==other.shape &&
                useShape==other.useShape &&
                displayEffects==other.displayEffects &&
                nodeProperties==other.nodeProperties;

            TRACE_EXIT_RET(retVal);
            return retVal;
        }

        NodePropertiesMessage &NodePropertiesMessage::operator=(const NodePropertiesMessage &other)
        {
            TRACE_ENTER();

            Message::operator=(other);
            layer=other.layer;
            color=other.color;
            useColor=other.useColor;
            size=other.size;
            shape=other.shape;
            useShape=other.useShape;
            displayEffects=other.displayEffects;
            nodeProperties=other.nodeProperties;

            TRACE_EXIT();
            return *this;
        }

        // virtual 
        std::ostream &NodePropertiesMessage::toStream(std::ostream &out) const
        {
            TRACE_ENTER();

            Message::toStream(out);
            out << " layer: " << layer;
            out << " color: " << color;
            out << " (" << (useColor?"active":"ignored") << ")";
            out << " size: " << size;
            out << " shape: " << nodeShapeToString(shape);
            out << " (" << (useShape?"active":"ignored") << ")";

            out << " effects: ";
            out << "[";
            BOOST_FOREACH(const DisplayEffect &e, displayEffects)
                out << displayEffectToString(e) << ",";
            out << "]";

            out << " properties: ";
            out << "[";
            BOOST_FOREACH(const NodeProperty &p, nodeProperties)
                out << nodePropertyToString(p) << ",";
            out << "]";

            TRACE_EXIT();
            return out;
        }

        ostream& operator<<(ostream &out, const NodePropertiesMessage &mess)
        {
            mess.operator<<(out);
            return out;
        }

		YAML::Emitter &NodePropertiesMessage::serialize(YAML::Emitter &e) const {
			e << YAML::Flow << YAML::BeginMap;
			Message::serialize(e); 
			e << YAML::Key << "layer" << YAML::Value << layer;
			e << YAML::Key << "color" << YAML::Value << color.toString();
			e << YAML::Key << "useColor" << YAML::Value << useColor;
			e << YAML::Key << "size" << YAML::Value << size;
			e << YAML::Key << "shape" << YAML::Value << static_cast<unsigned short>(shape);
			e << YAML::Key << "useShape" << YAML::Value << useShape;
			e << YAML::Key << "displayEffects" << YAML::Value; 
				e << YAML::Flow << YAML::BeginSeq; 
				BOOST_FOREACH(const NodePropertiesMessage::DisplayEffect &de, displayEffects) 
					e << de;
				e << YAML::EndSeq; 
			e << YAML::Key << "nodeProperties" << YAML::Value; 
				e << YAML::Flow << YAML::BeginSeq; 
				BOOST_FOREACH(const NodePropertiesMessage::NodeProperty &np, nodeProperties) 
					e << np;
				e << YAML::EndSeq; 
			e << YAML::EndMap; 
			return e; 
		}
		YAML::Node &NodePropertiesMessage::serialize(YAML::Node &node) {
			string str; 
			// Do not serialize base data GTL - Message::serialize(node); 
			node["layer"] >> layer;
			node["color"] >> str;
			color.fromString(str); 
			node["useColor"] >> useColor;
			node["size"] >> size;
			node["shape"] >> (unsigned short &)shape; 
			node["useShape"] >> useShape;
			const YAML::Node &deseq=node["displayEffects"]; 
			for (unsigned i=0;i<deseq.size();i++) {
				NodePropertiesMessage::DisplayEffect e; 
				deseq[i] >> (unsigned short &)e; 
				displayEffects.push_back(e); 
			}
			const YAML::Node &npseq=node["nodeProperties"]; 
			for (unsigned i=0;i<npseq.size();i++) {
				NodePropertiesMessage::NodeProperty e; 
				npseq[i] >> (unsigned short &)e; 
				nodeProperties.push_back(e); 
			}
			return node;
		}
        // static 
        string NodePropertiesMessage::nodeShapeToString(const NodePropertiesMessage::NodeShape &shape)
        {
            TRACE_ENTER();

            string retVal;
            switch(shape) 
            {
                case NOSHAPE: retVal="shapeless"; break;
                case CIRCLE: retVal="circle"; break;
                case SQUARE: retVal="square"; break;
                case TRIANGLE: retVal="triangle"; break;
                case TORUS: retVal="torus"; break;
                case TEAPOT: retVal="teapot"; break;
            }
            TRACE_EXIT_RET(retVal);
            return retVal;
        }

        // static 
        bool NodePropertiesMessage::stringToNodeShape(const string &s, NodePropertiesMessage::NodeShape &shape)
        {
            TRACE_ENTER();
            
            bool retVal=true;
            if (iequals(s, "shapeless")) shape=NOSHAPE;
            else if (iequals(s, "circle")) shape=CIRCLE;
            else if (iequals(s,"square")) shape=SQUARE;
            else if (iequals(s,"triangle")) shape=TRIANGLE;
            else if (iequals(s,"torus")) shape=TORUS;
            else if (iequals(s,"teapot")) shape=TEAPOT;
            else {
                LOG_ERROR("I don't know what shape " << shape << " represents, giving up.");
                retVal=false;
            }
            TRACE_EXIT_RET_BOOL(retVal);
            return retVal;
        }
        // static 
        std::string NodePropertiesMessage::nodePropertyToString(const NodePropertiesMessage::NodeProperty &p)
        {
            TRACE_ENTER();
            string retVal;
            switch(p) 
            {
                case NOPROPERTY: retVal="noproperty"; break;
                case LEAFNODE: retVal="leafnode"; break;
                case NEIGHBORHOOD: retVal="neighborhood"; break;
                case REGIONAL: retVal="regional"; break;
                case ROOT: retVal="root"; break;
                case ATTACKER: retVal="attacker"; break;
                case VICTIM: retVal="victim"; break;
                case CHOSEN: retVal="chosen"; break;
            }
            TRACE_EXIT_RET(retVal);
            return retVal;
        }
        // static
        bool NodePropertiesMessage::stringToNodeProperty(const std::string &s, NodePropertiesMessage::NodeProperty &p)
        {
            TRACE_ENTER();

            bool retVal=true;
            if (iequals(s, "noproperty")) p=NOPROPERTY;
            else if (iequals(s, "leafnode")) p=LEAFNODE;
            else if (iequals(s,"neighborhood")) p=NEIGHBORHOOD;
            else if (iequals(s,"regional")) p=REGIONAL;
            else if (iequals(s,"root")) p=ROOT;
            else if (iequals(s,"attacker")) p=ATTACKER;
            else if (iequals(s,"victim")) p=VICTIM;
            else if (iequals(s,"chosen")) p=CHOSEN;
            else  {
                LOG_ERROR("I don't know what property " << s << " represents."); 
                retVal=false;
            }
            TRACE_EXIT_RET_BOOL(retVal);
            return retVal;
        }
        // static 
        std::string NodePropertiesMessage::displayEffectToString(const NodePropertiesMessage::DisplayEffect &e)
        {
            TRACE_ENTER();
            string retVal;
            switch(e) 
            {
                case NOEFFECT: retVal="noeffect"; break;
                case SPARKLE: retVal="sparkle"; break;
                case SPIN: retVal="spin"; break;
                case FLASH: retVal="flash"; break;
            }
            TRACE_EXIT();
            return retVal;
        }

        // static 
        bool NodePropertiesMessage::stringToDisplayEffect(const std::string &s, NodePropertiesMessage::DisplayEffect &e)
        {
            TRACE_ENTER();
            bool retVal=true;
            if (iequals(s, "noeffect")) e=NodePropertiesMessage::NOEFFECT;
            else if (iequals(s, "sparkle")) e=NodePropertiesMessage::SPARKLE;
            else if (iequals(s,"spin")) e=NodePropertiesMessage::SPIN;
            else if (iequals(s,"flash")) e=NodePropertiesMessage::FLASH;
            else {
                LOG_ERROR("I don't know what effect " << e << " represents."); 
                retVal=false;
            }
            TRACE_EXIT_RET_BOOL(retVal);
            return retVal;
        }
    }
}

