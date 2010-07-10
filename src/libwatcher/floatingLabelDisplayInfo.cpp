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
#include "floatingLabelDisplayInfo.h"
#include "logger.h"

using namespace std;
using namespace watcher;
using namespace watcher::event;

INIT_LOGGER(FloatingLabelDisplayInfo, "DisplayInfo.LabelDisplayInfo.FloatingLabelDisplayInfo"); 

FloatingLabelDisplayInfo::FloatingLabelDisplayInfo() : 
    LabelDisplayInfo(),
    lat(0.0), 
    lng(0.0),
    alt(0.0)
{
    TRACE_ENTER();
    TRACE_EXIT();
}

FloatingLabelDisplayInfo::FloatingLabelDisplayInfo(const FloatingLabelDisplayInfo &copy) :
    LabelDisplayInfo(copy), 
    lat(copy.lat),
    lng(copy.lng),
    alt(copy.alt)
{

}

// virtual
FloatingLabelDisplayInfo::~FloatingLabelDisplayInfo()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

void FloatingLabelDisplayInfo::initialize(const watcher::event::LabelMessagePtr &m)
{
    LabelDisplayInfo::initialize(m); 
    lat=m->lat;
    lng=m->lng;
    alt=m->alt;
}

void FloatingLabelDisplayInfo::saveConfiguration() const
{
    LabelDisplayInfo::saveConfiguration();
    // NOOP for us.
}

FloatingLabelDisplayInfo &FloatingLabelDisplayInfo::operator=(const FloatingLabelDisplayInfo &lhs)
{
    LabelDisplayInfo::operator=(lhs);
    lat=lhs.lat;    
    lng=lhs.lng;    
    alt=lhs.alt;    
}

// virtual 
ostream &FloatingLabelDisplayInfo::toStream(ostream &out) const
{
    TRACE_ENTER();
    LabelDisplayInfo::toStream(out);
    out << "lng: " << lng << " lat: " << lat << " alt: " << alt;
    TRACE_EXIT();
    return out; 
}

ostream& watcher::operator<<(ostream &out, const FloatingLabelDisplayInfo &obj)
{
    obj.operator<<(out);
    return out;
}
