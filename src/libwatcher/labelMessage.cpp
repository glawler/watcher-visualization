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
 * @file labelMessage.h
 * @author Geoff Lawler <geoff.lawer@cobham.com>
 * @date 2009-07-15
 */
#include "labelMessage.h"
#include "marshalYAML.h"
#include "messageTypesAndVersions.h"
#include "colors.h"
#include "logger.h"

using namespace std;

namespace watcher {
    namespace event {
        INIT_LOGGER(LabelMessage, "Message.LabelMessage");

        LabelMessage::LabelMessage(const string &label_, int fontSize_)   :
            Message(LABEL_MESSAGE_TYPE, LABEL_MESSAGE_VERSION),
            label(label_),
            fontSize(fontSize_),
            foreground(colors::white),
            background(colors::black),
            expiration(0),
            addLabel(true),
            layer(UNDEFINED_LAYER),
            lat(0),
            lng(0),
            alt(0)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        LabelMessage::LabelMessage(const string &label_, const boost::asio::ip::address &address_, int fontSize_)   :
            Message(LABEL_MESSAGE_TYPE, LABEL_MESSAGE_VERSION),
            label(label_),
            fontSize(fontSize_),
            foreground(colors::white),
            background(colors::black),
            expiration(0),
            addLabel(true),
            layer(UNDEFINED_LAYER),
            lat(0),
            lng(0),
            alt(0)
        {
            TRACE_ENTER();
            fromNodeID=address_; 
            TRACE_EXIT();
        }

        LabelMessage::LabelMessage(const std::string &label_, const float &lat_, const float &lng_, const float &alt_, const int fontSize_) : 
            Message(LABEL_MESSAGE_TYPE, LABEL_MESSAGE_VERSION),
            label(label_),
            fontSize(fontSize_),
            foreground(colors::white),
            background(colors::black),
            expiration(0),
            addLabel(true),
            layer(UNDEFINED_LAYER), 
            lat(lat_),
            lng(lng_),
            alt(alt_)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        LabelMessage::LabelMessage(const LabelMessage &other) : 
            Message(other),
            label(other.label),
            fontSize(other.fontSize),
            foreground(other.foreground),
            background(other.background),
            expiration(other.expiration),
            addLabel(other.addLabel),
            layer(other.layer),
            lat(other.lat),
            lng(other.lng),
            alt(other.alt)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        bool LabelMessage::operator==(const LabelMessage &other) const
        {
            TRACE_ENTER();
 
            bool retVal = 
                    Message::operator==(other) && 
                    lat==other.lat && 
                    lng==other.lng &&
                    alt==other.alt &&
                    label==other.label &&
                    layer==other.layer;

            // These are not distinguishing features
            //  foreground==other.foreground,
            //  background==other.background,
            //  expiration==other.expiration,
            //  fontSize==other.FontSize;

            TRACE_EXIT_RET(retVal);
            return retVal;
        }

        LabelMessage &LabelMessage::operator=(const LabelMessage &other)
        {
            TRACE_ENTER();

            Message::operator=(other);
            label=other.label; 
            fontSize=other.fontSize;
            foreground=other.foreground;
            background=other.background;
            expiration=other.expiration;
            addLabel=other.addLabel;
            layer=other.layer;
            lat=other.lat;
            lng=other.lng;
            alt=other.alt;

            TRACE_EXIT();
            return *this;
        }

        // virtual 
        std::ostream &LabelMessage::toStream(std::ostream &out) const
        {
            TRACE_ENTER();

            Message::toStream(out);
            out << " label: " << label;
            if (lat!=0.0 && lng!=0.0)
                out << " (floating) ";
            else
                out << " (attached) "; 
            out << " font size: " << fontSize; 
            out << " layer: " << layer; 
            out << " fg: (" << foreground << ")"; 
            out << " bg: (" << background << ")"; 
            out << " exp: " << expiration;
            out << " add: " << (addLabel ? "true" : "false"); 
            out << " lat: " << lat; 
            out << " lng: " << lng; 
            out << " alt: " << alt; 

            TRACE_EXIT();
            return out;
        }

        ostream& operator<<(ostream &out, const LabelMessage &mess)
        {
            mess.operator<<(out);
            return out;
        }

		YAML::Emitter &LabelMessage::serialize(YAML::Emitter &e) const {
			e << YAML::Flow << YAML::BeginMap;
			Message::serialize(e); 
			e << YAML::Key << "label" << YAML::Value << label;
			e << YAML::Key << "foreground" << YAML::Value << foreground.toString(); 
			e << YAML::Key << "background" << YAML::Value << background.toString(); 
			e << YAML::Key << "expiration" << YAML::Value << expiration;
			e << YAML::Key << "fontSize" << YAML::Value << fontSize;
			e << YAML::Key << "addLabel" << YAML::Value << addLabel;
			e << YAML::Key << "layer" << YAML::Value << layer;
			e << YAML::Key << "lat" << YAML::Value << lat;
			e << YAML::Key << "lng" << YAML::Value << lng;
			e << YAML::Key << "alt" << YAML::Value << alt;
			e << YAML::EndMap; 
			return e; 
		}
		YAML::Node &LabelMessage::serialize(YAML::Node &node) {
			// Do not serialize base data GTL - Message::serialize(node); 
			string str; 
			node["label"] >> label;
			node["foreground"] >> str;
			foreground.fromString(str); 
			node["background"] >> str;
			background.fromString(str); 
			node["expiration"] >> expiration;
			node["fontSize"] >> fontSize;
			node["addLabel"] >> addLabel;
			node["layer"] >> layer;
			node["lat"] >> lat;
			node["lng"] >> lng;
			node["alt"] >> alt;
			return node;
		}
    }
}

