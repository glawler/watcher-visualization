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
            size(1.0),
            shape(CIRCLE),
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
            size(other.size),
            shape(other.shape),
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
                size==other.size &&
                shape==other.shape &&
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
            size=other.size;
            shape=other.shape;
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
            out << " size: " << size;
            out << " shape: " << nodeShapeToString(shape);

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
            ar & size;
            ar & shape;
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
        NodePropertiesMessage::NodeShape NodePropertiesMessage::stringToNodeShape(const string &shape)
        {
            TRACE_ENTER();
            NodeShape retVal;
            if (iequals(shape, "circle")) retVal=CIRCLE;
            else if (iequals(shape,"square")) retVal=SQUARE;
            else if (iequals(shape,"triangle")) retVal=TRIANGLE;
            else if (iequals(shape,"torus")) retVal=TORUS;
            else if (iequals(shape,"teapot")) retVal=TEAPOT;
            else
                LOG_ERROR("I don't know what shape " << shape << " represents, guessing circle"); 
            TRACE_EXIT();
            return retVal;
        }
        // static 
        std::string NodePropertiesMessage::nodePropertyToString(const NodePropertiesMessage::NodeProperty &p)
        {
            TRACE_ENTER();
            string retVal;
            switch(p) 
            {
                case NodePropertiesMessage::LEAFNODE: retVal="leafnode"; break;
                case NodePropertiesMessage::CLUSTERHEAD: retVal="clusterhead"; break;
                case NodePropertiesMessage::ROOT: retVal="root"; break;
                case NodePropertiesMessage::ATTACKER: retVal="attacker"; break;
                case NodePropertiesMessage::VICTIM: retVal="victim"; break;
            }
            TRACE_EXIT_RET(retVal);
            return retVal;
        }
        // static
        NodePropertiesMessage::NodeProperty NodePropertiesMessage::stringToNodeProperty(const std::string &shape)
        {
            TRACE_ENTER();
            NodePropertiesMessage::NodeProperty retVal;
            if (iequals(shape, "leafnode")) retVal=NodePropertiesMessage::LEAFNODE;
            else if (iequals(shape,"clusterhead")) retVal=NodePropertiesMessage::CLUSTERHEAD;
            else if (iequals(shape,"root")) retVal=NodePropertiesMessage::ROOT;
            else if (iequals(shape,"attacker")) retVal=NodePropertiesMessage::ATTACKER;
            else if (iequals(shape,"victim")) retVal=NodePropertiesMessage::VICTIM;
            else 
                LOG_ERROR("I don't know what property " << shape << " represents."); 
            TRACE_EXIT();
            return retVal;
        }
        // static 
        std::string NodePropertiesMessage::displayEffectToString(const NodePropertiesMessage::DisplayEffect &e)
        {
            TRACE_ENTER();
            string retVal;
            switch(e) 
            {
                case NodePropertiesMessage::SPARKLE: retVal="sparkle"; break;
                case NodePropertiesMessage::SPIN: retVal="spin"; break;
                case NodePropertiesMessage::FLASH: retVal="flash"; break;
            }
            TRACE_EXIT_RET(retVal);
            return retVal;
        }
        // static 
        NodePropertiesMessage::DisplayEffect NodePropertiesMessage::stringToDisplayEffect(const std::string &e)
        {
            TRACE_ENTER();
            NodePropertiesMessage::DisplayEffect retVal;
            if (iequals(e, "sparkle")) retVal=NodePropertiesMessage::SPARKLE;
            else if (iequals(e,"spin")) retVal=NodePropertiesMessage::SPIN;
            else if (iequals(e,"flash")) retVal=NodePropertiesMessage::FLASH;
            else 
                LOG_ERROR("I don't know what effect " << e << " represents."); 
            TRACE_EXIT();
            return retVal;
        }
    }
}

BOOST_CLASS_EXPORT(watcher::event::NodePropertiesMessage);


