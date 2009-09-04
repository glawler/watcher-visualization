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

#include "watcherSerialize.h"
#include "nodePropertiesMessage.h"
#include "messageTypesAndVersions.h"
#include "colors.h"

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

            out << "[";
            BOOST_FOREACH(const DisplayEffect &e, displayEffects)
                out << displayEffectToString(e) << ",";
            out << "]";

            out << "[";
            BOOST_FOREACH(const NodeProperty &p, nodeProperties)
                out << nodePropertyToString(p) << ",";
            out << "]";

            TRACE_EXIT();
            return out;
        }

        ostream& operator<<(ostream &out, const NodePropertiesMessage &mess)
        {
            TRACE_ENTER();
            mess.operator<<(out);
            TRACE_EXIT();
            return out;
        }

        template <typename Archive> void NodePropertiesMessage::serialize(Archive & ar, const unsigned int /* file_version */)
        {
            TRACE_ENTER();
            ar & boost::serialization::base_object<Message>(*this);
            ar & layer;
            ar & color;
            ar & useColor;
            ar & size;
            ar & shape;
            ar & useShape;
            ar & displayEffects;
            ar & nodeProperties;
            TRACE_EXIT();
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
                case CLUSTERHEAD: retVal="clusterhead"; break;
                case ROOT: retVal="root"; break;
                case ATTACKER: retVal="attacker"; break;
                case VICTIM: retVal="victim"; break;
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
            else if (iequals(s,"clusterhead")) p=CLUSTERHEAD;
            else if (iequals(s,"root")) p=ROOT;
            else if (iequals(s,"attacker")) p=ATTACKER;
            else if (iequals(s,"victim")) p=VICTIM;
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

BOOST_CLASS_EXPORT(watcher::event::NodePropertiesMessage);


