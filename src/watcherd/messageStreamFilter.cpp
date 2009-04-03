#include "messageStreamFilter.h"

using namespace watcher;


MessageStreamFilter::MessageStreamFilter() : 
            layer(""), messageType(0), region()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

MessageStreamFilter::~MessageStreamFilter()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

//virtual 
std::ostream &MessageStreamFilter::toStream(std::ostream &out) const
{
    TRACE_ENTER();
    out << "layer: " << layer 
        << ", type: " << messageType 
        << ", region: " << region; 
    TRACE_EXIT();
    return out; 
}


std::ostream &operator<<(std::ostream &out, const MessageStreamFilter &messStreamFilter)
{
    TRACE_ENTER();
    TRACE_EXIT();
    return messStreamFilter.operator<<(out);
}


