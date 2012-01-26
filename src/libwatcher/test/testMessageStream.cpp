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
 * @file testMessageStream.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
#include <stdlib.h>

#define BOOST_TEST_MODULE watcher::messageStream test
#include <boost/test/unit_test.hpp>

#include "../messageStream.h"

using namespace std;
using namespace boost;
using namespace watcher;
using namespace boost::unit_test_framework;


BOOST_AUTO_TEST_CASE( ctors_test )
{
    // Don't actually do this cause it needs a running watcherd 
    // MessageStreamPtr ms=MessageStream::createNewMessageStream("glory"); 
}



