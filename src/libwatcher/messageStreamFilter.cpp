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
#include "message.h"
#include "labelMessage.h"
#include "edgeMessage.h"
#include "connectivityMessage.h"
#include "colorMessage.h"
#include "nodePropertiesMessage.h"
#include "logger.h"

using namespace watcher;

INIT_LOGGER(MessageStreamFilter, "MessageStreamFilter");

MessageStreamFilter::MessageStreamFilter() : 
            layer(), messageType(0), region()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

MessageStreamFilter::MessageStreamFilter(const MessageStreamFilter &copyme)
{
    TRACE_ENTER();
    *this=copyme;
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

bool MessageStreamFilter::operator==(const MessageStreamFilter &other) const 
{
    TRACE_ENTER();
    bool retVal=
        layer==other.layer && 
        messageType==other.messageType && 
        region==other.region;

    TRACE_EXIT_RET_BOOL(retVal);
    return retVal;
}

MessageStreamFilter &MessageStreamFilter::operator=(const MessageStreamFilter &other) 
{
    TRACE_ENTER();
    layer=other.layer;
    messageType=other.messageType;
    region=other.region;
    TRACE_EXIT();
}

bool MessageStreamFilter::passFilter(const MessagePtr m) const
{
    TRACE_ENTER();
    // Really need to make layers a member of a base class...
    bool isMessageType=false, isLayer=false;
    if (messageType) 
       isMessageType=messageType==m->type; 
    if (!layer.empty()) { 
        switch (m->type)
        {
            case LABEL_MESSAGE_TYPE: 
                isLayer=layer==(boost::dynamic_pointer_cast<LabelMessage>(m))->layer; 
                break;
            case EDGE_MESSAGE_TYPE: 
                isLayer=layer==(boost::dynamic_pointer_cast<EdgeMessage>(m))->layer; 
                break;
            case COLOR_MESSAGE_TYPE: 
                isLayer=layer==(boost::dynamic_pointer_cast<ColorMessage>(m))->layer; 
                break;
            case CONNECTIVITY_MESSAGE_TYPE: 
                isLayer=layer==(boost::dynamic_pointer_cast<ConnectivityMessage>(m))->layer; 
                break;
            case NODE_PROPERTIES_MESSAGE_TYPE: 
                isLayer=layer==(boost::dynamic_pointer_cast<NodePropertiesMessage>(m))->layer; 
                break;
            default: 
                break;
        }
    }
    
    bool retVal=(isMessageType||isLayer);
    TRACE_EXIT_RET_BOOL(retVal);
    return retVal;
}

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


std::ostream &watcher::operator<<(std::ostream &out, const MessageStreamFilter &messStreamFilter)
{
    return messStreamFilter.operator<<(out);
}


