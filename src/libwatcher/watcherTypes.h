/**
 * @file watcherTypes.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
#ifndef WATCHER_TYPES_HELLO_THERE_H
#define WATCHER_TYPES_HELLO_THERE_H

#include <boost/asio.hpp>
#include <boost/asio/ip/address.hpp>

/** Toplevel namespace for the Watcher project */
namespace watcher 
{
    /** 
     * used to hide the implementation of how to uniquely identify a node. 
     */ 
    typedef boost::asio::ip::address NodeIdentifier;

    /**
     * Give timestamp its own type - it's Unix epoch milliseconds 
     */
    typedef long long int Timestamp;    // in Epoch milliseconds

    const Timestamp Infinity = -1;

    /**
     * Return the current time in milliseconds, using typedef type.
     * @return current time in milliseconds
     */
    Timestamp getCurrentTime();
}

#endif // WATCHER_TYPES_HELLO_THERE_H
