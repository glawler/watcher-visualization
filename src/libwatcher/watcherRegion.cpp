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

