#include "watcherRegion.h"

using namespace watcher;

INIT_LOGGER(WatcherRegion, "WatcherRegion");

WatcherRegion::WatcherRegion()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

// virtual
WatcherRegion::~WatcherRegion()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

// virtual 
std::ostream &WatcherRegion::toStream(std::ostream &out) const
{
    TRACE_ENTER();
    TRACE_EXIT();
    return out; 
}

std::ostream &watcher::operator<<(std::ostream &out, const WatcherRegion &region)
{
    TRACE_ENTER();
    TRACE_EXIT();
    return region.operator<<(out);
}

