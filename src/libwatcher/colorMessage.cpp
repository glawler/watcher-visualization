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
 * @file colorMessage.cpp
 * @author Geoff Lawler <geoff.lawer@cobham.com>
 * @date 2009-07-15
 */
#include "watcherGlobalFunctions.h" // for address serialization
#include "colorMessage.h"
#include "colors.h"
#include "logger.h"
#include "marshal.hpp"

using namespace std;
using namespace boost;

namespace watcher {
    using namespace colors;
    namespace event {
        INIT_LOGGER(ColorMessage, "Message.ColorMessage");

        ColorMessage::ColorMessage() : 
            Message(COLOR_MESSAGE_TYPE, COLOR_MESSAGE_VERSION),
            color(black),
            flashPeriod(0),
            expiration(0),
            layer(PHYSICAL_LAYER)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }


        ColorMessage::ColorMessage(
                                   const Color &c, 
                                   const boost::asio::ip::address &address,
                                   const Timestamp &e,
                                   const Timestamp &f) : 
            Message(COLOR_MESSAGE_TYPE, COLOR_MESSAGE_VERSION),
            color(c), 
            flashPeriod(e),
            expiration(f),
            layer(PHYSICAL_LAYER)
        {
            TRACE_ENTER();
            fromNodeID=address;
            TRACE_EXIT();
        }

        ColorMessage::ColorMessage(const ColorMessage &other) : 
            Message(other.type, other.version),
            color(other.color), 
            flashPeriod(other.flashPeriod),
            expiration(other.expiration),
            layer(other.layer)
        {
            TRACE_ENTER();
            (*this)=other;
            TRACE_EXIT();
        }

        bool ColorMessage::operator==(const ColorMessage &other) const
        {
            TRACE_ENTER();

            bool retVal = 
                Message::operator==(other) && 
                color == other.color && 
                layer == other.layer;

            TRACE_EXIT_RET(retVal);
            return retVal;
        }

        ColorMessage &ColorMessage::operator=(const ColorMessage &other)
        {
            TRACE_ENTER();

            Message::operator=(other);
            color = other.color;
            flashPeriod = other.flashPeriod;
            expiration = other.expiration;
            layer = other.layer; 

            TRACE_EXIT();
            return *this;
        }

        // virtual 
        std::ostream &ColorMessage::toStream(std::ostream &out) const
        {
            TRACE_ENTER();

            Message::toStream(out);
            out << " color: " << color; 
            out << " flashPeriod: " << flashPeriod;
            out << " expiration: " << expiration;
            out << " layer: " << layer; 
            TRACE_EXIT();
            return out;
        }

        ostream& operator<<(ostream &out, const ColorMessage &mess)
        {
            mess.operator<<(out);
            return out;
        }

        //manipulator to deserialize a color
        Marshal::Input& operator>> (Marshal::Input& in, Color& c)
        {
            std::string s;
            in >> s;
            c.fromString(s);
            return in;
        }

        void ColorMessage::readPayload(std::istream& is)
        {
            TRACE_ENTER();
            Marshal::Input in(is);
            in >> color;
            in >> flashPeriod;
            in >> expiration;
            in >> layer; 
            TRACE_EXIT();
        }
    }
}
