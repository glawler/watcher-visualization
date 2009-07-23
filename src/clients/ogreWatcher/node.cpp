/**
 * @file node.cpp 
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-20
 */
#include <Ogre.h>
#include "node.h"

using namespace Ogre;

namespace ogreWatcher
{
    INIT_LOGGER(node, "node"); 

    node::node()
    {
        TRACE_ENTER();
        TRACE_EXIT();
    }

    // virtual 
    node::~node()
    {
        TRACE_ENTER();
        TRACE_EXIT();
    }
}

