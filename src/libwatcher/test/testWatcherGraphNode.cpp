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
 * @file testWatcherGraphNode.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
#define BOOST_TEST_MODULE watcher::watcherGraphNode test
#include <boost/test/unit_test.hpp>

#include "../watcherGraphNode.h"

using namespace std;
using namespace boost;
using namespace watcher;
using namespace watcher::event;
using namespace boost::unit_test_framework;

BOOST_AUTO_TEST_CASE( ctor_test )
{
    WatcherGraphNode wgn;
}

BOOST_AUTO_TEST_CASE( output_test )
{
    BOOST_TEST_MESSAGE("Checking Graph Node output operator,"); 

    WatcherGraphNode wgn;

    cout << "Empty Node:" << endl << wgn << endl;

    wgn.nodeId.from_string("192.168.1.123");
    wgn.gpsData=GPSMessagePtr(new GPSMessage(0.1234, 0.2345, 0.3456));
    wgn.displayInfo->label="This is a label there are others like it, but this one is mine."; 
    wgn.connected=true;

    // Create from label message
    LabelMessagePtr lmp(new LabelMessage("Hello"));
    lmp->layer="twilight zone";
    LabelDisplayInfoPtr ldip(new LabelDisplayInfo); 
    ldip->loadConfiguration(lmp); 
    wgn.labels.push_back(ldip);

    // create from label display info
    ldip.reset(new LabelDisplayInfo);
    ldip->layer="chuck e. cheeze";
    ldip->labelText="From the FUN zone!"; 
    wgn.labels.push_back(ldip);

    cout << "Node with stuff in it: " << endl << wgn << endl;
}


