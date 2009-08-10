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

#include "messageStreamFilter.h"

using namespace watcher;

INIT_LOGGER(MessageStreamFilter, "MessageStreamFilter");

MessageStreamFilter::MessageStreamFilter() : 
            layer(), messageType(0), region()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

MessageStreamFilter::~MessageStreamFilter()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

GUILayer MessageStreamFilter::getLayer() const { return layer; } 
void MessageStreamFilter::setLayer(const GUILayer &l) { layer=l; }
unsigned int MessageStreamFilter::getMessageType() const { return messageType; } 
void MessageStreamFilter::setMessageType(const unsigned int &t) { messageType=t; }
WatcherRegion MessageStreamFilter::getRegion() const { return region; } 
void MessageStreamFilter::setRegion(const WatcherRegion &r) { region=r; } 

//virtual 
std::ostream &MessageStreamFilter::toStream(std::ostream &out) const
{
    TRACE_ENTER();
    out << " layer: " << layer 
        << " type: " << messageType 
        << " region: " << region; 
    TRACE_EXIT();
    return out; 
}


std::ostream &operator<<(std::ostream &out, const MessageStreamFilter &messStreamFilter)
{
    TRACE_ENTER();
    TRACE_EXIT();
    return messStreamFilter.operator<<(out);
}


