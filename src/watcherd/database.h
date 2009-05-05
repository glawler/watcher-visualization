/**@file
 * @author Michael.Elkins@cobham.com
 * @date 2009-05-04
 */

#ifndef database_h
#define database_h

#include <boost/utility.hpp>
#include <string>

#include "libwatcher/message_fwd.h"

namespace watcher {
    /** Abstract class used to provide an interface to a database backend for storing event streams. */
    class Database : private boost::noncopyable {
        public:
            static Database* connect(const std::string&);

            /** Store an event received from a specified host into the database.
             * @param[in] addr IP address of the node from which the event was received
             * @param[in] msg the Event to store
             */
            virtual void storeEvent(const std::string& addr, event::MessagePtr msg) = 0;

            virtual ~Database() = 0;
    };
} //namespace

#endif /* database_h */
