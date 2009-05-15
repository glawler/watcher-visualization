/**@file
 * @author Michael.Elkins@cobham.com
 * @date 2009-05-04
 */

#ifndef database_h
#define database_h

#include <boost/utility.hpp>
#include <boost/function.hpp>
#include <string>

#include "libwatcher/message_fwd.h"
#include "libwatcher/watcherTypes.h"

namespace watcher {
    /** Abstract class used to provide an interface to a database backend for
     * storing event streams. */
    class Database : private boost::noncopyable {
        public:
            static Database* connect(const std::string&);

            /** Store an event received from a specified host into the database.
             *
             * @param[in] addr IP address of the node from which the event was received
             * @param[in] msg the Event to store
             */
            virtual void storeEvent(const std::string& addr, event::MessagePtr msg) = 0;


            enum Direction { forward, reverse };

            /** Retreive events from the database.
             * @param output a function which accepts the individual events returned from the DB
             * @param[in] t time offset at which to start retrieving events
             * @param[in] d direction of the event stream
             * @param[in] count the max number of events to retrieve
             */
            virtual void getEvents( boost::function<void(event::MessagePtr)> output, Timestamp t, Direction d, unsigned int count ) = 0;

            virtual ~Database() = 0;
    };

    /** Get the thread-local database handle for the current thread.  If one
     * does not exist, it is created.  The URI for the database is found from
     * the SingletonConfig instance for the process.
     *
     * @return handle to thread-local database handle.
     */
    Database& get_db_handle();

} //namespace

#endif /* database_h */
