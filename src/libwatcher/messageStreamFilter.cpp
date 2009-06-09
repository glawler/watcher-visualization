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


