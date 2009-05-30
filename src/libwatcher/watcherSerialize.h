#ifndef watcher_serialize_h
#define watcher_serialize_h

/** @file
 *
 * This file contains the appropriate Boost header files required to serialize
 * message.  It should be included in all .cpp files that include
 * BOOST_CLASS_EXPORT.
 *
 * @author Michael Elkins <Michael.Elkins@cobham.com>
 * @date 2009-03-20
 */

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/graph/adj_list_serialize.hpp>   // for serialing boost::graphs. 
#include <boost/serialization/string.hpp>       // for serializing addresses.
#include <boost/serialization/vector.hpp>       // for serializing addresses.
#include <boost/serialization/shared_ptr.hpp>   // for serializing LabelMessagePtrs
#include <boost/serialization/list.hpp>
#include <boost/serialization/array.hpp>        // address.v4 bytes is an array of char
#include <boost/serialization/export.hpp>

#include "watcherGlobalFunctions.h"     // for address serialization

#endif /* watcher_serialize_h */
