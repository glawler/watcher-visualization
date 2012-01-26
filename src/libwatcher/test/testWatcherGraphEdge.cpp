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
 * @file testWatcherGraphEdge.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
#define BOOST_TEST_MODULE watcher::watcherGraphEdge test
#include <boost/test/unit_test.hpp>

#include "../watcherGraphEdge.h"

using namespace std;
using namespace boost;
using namespace watcher;
using namespace watcher::event;
using namespace boost::unit_test_framework;


BOOST_AUTO_TEST_CASE( ctor_test )
{
    WatcherGraphEdge wge;
}

BOOST_AUTO_TEST_CASE( output_test )
{
    BOOST_TEST_MESSAGE("Checking Graph Edge output operator,"); 

    WatcherGraphEdge wge;

    cout << "Empty Edge:" << endl << wge << endl;

    wge.displayInfo->loadConfiguration("Bogus Layer"); 
    wge.displayInfo->label="This is an edge. There are others like it, but this on is mine."; 
    wge.displayInfo->color=colors::yellow;
    wge.displayInfo->width=12.232412;
    wge.expiration=10000;

    struct 
    {
        char *label;
        char *layer;
    } data [] = {
        { "Hello" , "hello layer" }, 
        { "World" , "World layer" }, 
        { "doodoodoodoodoodoodoodoo" , "twilight zone" }, 
        { "Coffee_Cups" , "Bed, Bath, and, Beyond" }
    };

    for(unsigned int i=0; i< (sizeof(data)/sizeof(data[0])); i++)
    {
        LabelMessagePtr lmp(new LabelMessage(data[i].label));
        lmp->layer=data[i].layer;
        LabelDisplayInfoPtr ldip(new LabelDisplayInfo); 
        ldip->loadConfiguration(lmp); 
        wge.labels.push_back(ldip);
    }

    cout << "Edge with stuff in it: " << wge << endl;
}


