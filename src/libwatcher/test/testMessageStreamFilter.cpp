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
 * @file testMessageStreamFilter.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2010-15-10
 */
#include <stdlib.h>

#define BOOST_TEST_MODULE watcher::messageStreamFilter test
#include <boost/test/unit_test.hpp>

#include "../messageStreamFilter.h"
#include "../labelMessage.h"
#include "../connectivityMessage.h"
#include "../edgeMessage.h"

using namespace std;
using namespace boost;
using namespace watcher;
using namespace watcher::event;
using namespace boost::unit_test_framework;

BOOST_AUTO_TEST_CASE( ctors_test )
{
    MessageStreamFilterPtr f=MessageStreamFilterPtr(new MessageStreamFilter);
}

BOOST_AUTO_TEST_CASE( and_or_filter )
{
    LabelMessagePtr lm=LabelMessagePtr(new LabelMessage);
    lm->layer="layerOne";

    ConnectivityMessagePtr cm=ConnectivityMessagePtr(new ConnectivityMessage); 
    cm->layer="layerOne"; 

    ConnectivityMessagePtr cmDiff=ConnectivityMessagePtr(new ConnectivityMessage); 
    cmDiff->layer="layerTwo"; 

    EdgeMessagePtr em=EdgeMessagePtr(new EdgeMessage);  
    em->layer="layerTwo"; 

    // test AND
    MessageStreamFilterPtr f=MessageStreamFilterPtr(new MessageStreamFilter(true)); 
    f->addMessageType(cm->type);   
    f->addLayer(cm->layer);      
    BOOST_CHECK_EQUAL(true, f->passFilter(cm));         // same layer same type
    BOOST_CHECK_EQUAL(false, f->passFilter(lm));        // same layer diff type
    BOOST_CHECK_EQUAL(false, f->passFilter(cmDiff));    // diff layer same type
    BOOST_CHECK_EQUAL(false, f->passFilter(em));        // diff layer diff type

    // test OR  
    f=MessageStreamFilterPtr(new MessageStreamFilter(false)); 
    f->addMessageType(cm->type); 
    f->addLayer(lm->layer);     
    BOOST_CHECK_EQUAL(true, f->passFilter(cm));         // same layer same type
    BOOST_CHECK_EQUAL(true, f->passFilter(lm));         // same layer diff type
    BOOST_CHECK_EQUAL(true, f->passFilter(cmDiff));     // diff layer same type
    BOOST_CHECK_EQUAL(false, f->passFilter(em));        // diff layer diff type
}

BOOST_AUTO_TEST_CASE( multi_value_filter )
{
    LabelMessagePtr lm=LabelMessagePtr(new LabelMessage);
    lm->layer="layerOne";

    ConnectivityMessagePtr cm=ConnectivityMessagePtr(new ConnectivityMessage); 
    cm->layer="layerOne"; 

    EdgeMessagePtr em=EdgeMessagePtr(new EdgeMessage);  
    em->layer="layerTwo"; 

    // test AND
    MessageStreamFilterPtr f=MessageStreamFilterPtr(new MessageStreamFilter(true)); 
    f->addMessageType(cm->type);   
    f->addMessageType(lm->type);   
    f->addMessageType(em->type);   

    BOOST_CHECK_EQUAL(false, f->passFilter(lm));    // AND cannot match all types 
    BOOST_CHECK_EQUAL(false, f->passFilter(cm));    // AND cannot match all types 
    BOOST_CHECK_EQUAL(false, f->passFilter(em));    // AND cannot match all types 

    // test OR  
    f=MessageStreamFilterPtr(new MessageStreamFilter(false)); 
    f->addMessageType(cm->type);   
    f->addMessageType(lm->type);   
    f->addMessageType(em->type);   

    BOOST_CHECK_EQUAL(true, f->passFilter(lm));    // OR matches lm->type
    BOOST_CHECK_EQUAL(true, f->passFilter(cm));    // OR matches cm->type
    BOOST_CHECK_EQUAL(true, f->passFilter(em));    // OR matches em->type
}
BOOST_AUTO_TEST_CASE( layer_filter )
{
    MessageStreamFilterPtr f=MessageStreamFilterPtr(new MessageStreamFilter);
    f->addLayer("FauxLayer"); 

    LabelMessagePtr lm=LabelMessagePtr(new LabelMessage("HelloWorld")); 
    lm->layer="NotTheRightLayer"; 
    BOOST_CHECK_EQUAL(false, f->passFilter(lm)); 
    
    lm->layer="FauxLayer"; 
    BOOST_CHECK_EQUAL(true, f->passFilter(lm)); 
}

BOOST_AUTO_TEST_CASE( message_type_filter )
{
    ConnectivityMessagePtr cm=ConnectivityMessagePtr(new ConnectivityMessage); 

    MessageStreamFilterPtr f=MessageStreamFilterPtr(new MessageStreamFilter);
    f->addMessageType(cm->type); 

    BOOST_CHECK_EQUAL(true, f->passFilter(cm)); 

    LabelMessagePtr lm=LabelMessagePtr(new LabelMessage); 
    BOOST_CHECK_EQUAL(false, f->passFilter(lm)); 

}
