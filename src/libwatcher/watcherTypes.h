#ifndef WATCHER_TYPES_HELLO_THERE_H
#define WATCHER_TYPES_HELLO_THERE_H

#include <boost/asio.hpp>
#include <boost/asio/ip/address.hpp>

namespace watcher 
{
    /** 
     * @typedef nodeIdentifer is used to hide the implementation of how to uniquely identify a node. 
     */ 
    typedef boost::asio::ip::address NodeIdentifier;

    /**
     * @typedef Give timestamp its own type - it's Unix epoch milliseconds 
     */
    typedef long long int Timestamp;    // in Epoch milliseconds
}

#endif // WATCHER_TYPES_HELLO_THERE_H
