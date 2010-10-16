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

MessageStreamFilter::MessageStreamFilter(bool op) : 
            layers(), messageTypes(), /* region(), */ opAND(op)
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

void MessageStreamFilter::addLayer(const GUILayer &l) { layers.push_back(l); }
void MessageStreamFilter::addMessageType(const unsigned int &t) { messageTypes.push_back(t); }
// void MessageStreamFilter::addRegion(const WatcherRegion &r) { region=r; } 

bool MessageStreamFilter::operator==(const MessageStreamFilter &other) const 
{
    TRACE_ENTER();
    bool retVal=
        layers==other.layers && 
        messageTypes==other.messageTypes && 
        // region==other.region &&
        opAND==other.opAND;

    TRACE_EXIT_RET_BOOL(retVal);
    return retVal;
}

MessageStreamFilter &MessageStreamFilter::operator=(const MessageStreamFilter &other) 
{
    TRACE_ENTER();
    layers=other.layers;
    messageTypes=other.messageTypes;
    // region=other.region;
    opAND=other.opAND;
    TRACE_EXIT();
}

bool MessageStreamFilter::passFilter(const MessagePtr m) const
{
    TRACE_ENTER();
    
    // If we're in AND mode, then one false --> return false. 
    // If we're in AND mode and we make it to the end, all are true, return true
    // If we're in OR mode, then one true --> return true.
    // If we're in OR mode and we make it to the end all are false, return false

    if (messageTypes.size()) 
        for (std::vector<unsigned int>::const_iterator t=messageTypes.begin(); t!=messageTypes.end(); t++) {
            if (opAND) {
                if ((*t)!=(unsigned int)m->type)
                    return false; 
            }
            else {
                if ((*t)==(unsigned int)m->type)
                    return true;
            }
        }

    if (layers.size()) {
        std::string layer;
        // Really need to make layers a member of a base class...
        switch (m->type)
        {
            case LABEL_MESSAGE_TYPE: 
                layer=(boost::dynamic_pointer_cast<LabelMessage>(m))->layer; 
                break;
            case EDGE_MESSAGE_TYPE: 
                layer=(boost::dynamic_pointer_cast<EdgeMessage>(m))->layer; 
                break;
            case COLOR_MESSAGE_TYPE: 
                layer=(boost::dynamic_pointer_cast<ColorMessage>(m))->layer; 
                break;
            case CONNECTIVITY_MESSAGE_TYPE: 
                layer=(boost::dynamic_pointer_cast<ConnectivityMessage>(m))->layer; 
                break;
            case NODE_PROPERTIES_MESSAGE_TYPE: 
                layer=(boost::dynamic_pointer_cast<NodePropertiesMessage>(m))->layer; 
                break;
            default: 
                break;
        }
        if (layer.size()) 
            for (std::vector<std::string>::const_iterator l=layers.begin(); l!=layers.end(); l++) {
                if (opAND) {
                    if (layer!=*l)
                        return false;
                } 
                else {
                    if (layer==*l)
                        return true;
                }
            }
    }

    bool retVal=opAND==true?true:false;  // could just return opAND here but may be confusing. Compiler may take care of it.
    TRACE_EXIT_RET_BOOL(retVal);
    return retVal;
}

//virtual 
std::ostream &MessageStreamFilter::toStream(std::ostream &out) const
{
    TRACE_ENTER();
    out << " layers (" << layers.size() << "): ";
    std::copy(layers.begin(), layers.end(), std::ostream_iterator<std::string>(out, " "));
    out << ", message types:  (" << messageTypes.size() << "): ";
    std::copy(messageTypes.begin(), messageTypes.end(), std::ostream_iterator<unsigned int>(out, " "));
    // << " region: " << region
    out << " op: " << (opAND==true?"AND":"OR");
    TRACE_EXIT();
    return out; 
}

std::ostream &watcher::operator<<(std::ostream &out, const MessageStreamFilter &messStreamFilter)
{
    return messStreamFilter.operator<<(out);
}


