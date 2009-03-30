#ifndef WATCHER_TYPES_HELLO_THERE_H
#define WATCHER_TYPES_HELLO_THERE_H

#include <boost/asio.hpp>

namespace watcher 
{
    namespace watchapi 
    {
        /** 
         * nodeIdentifer is used to hide the implementation of how to uniquely identify a node. 
         */ 
        typedef boost::asio::ip::address NodeIdentifier;

        const NodeIdentifier Watchers;   /// Watchers
    }
}

#endif // WATCHER_TYPES_HELLO_THERE_H
