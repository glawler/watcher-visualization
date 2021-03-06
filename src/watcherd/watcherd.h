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
 * @page watcherd
 *
 * Main classes implementing the Watcher Daemon (watcherd):
 *      - @ref watcher::Watcherd
 *      - @ref watcher::Server
 *      - @ref watcher::ServerConnection
 *      - @ref watcher::Database
 *      - @ref watcher::SqliteDatabase
 *      - @ref watcher::ReplayState
 */

#ifndef WATCHERD_H
#define WATCHERD_H

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/utility.hpp>
#include <boost/thread.hpp>

#include <libwatcher/watcherMessageFwd.h>

#include "watcherd_fwd.h"
#include "serverMessageHandler.h"
#include "server.h"
#include "libconfig.h++"
#include "declareLogger.h"
#include "sharedStreamFwd.h"

namespace watcher
{
    /// A class representing the instance of a watcher daemon
    class Watcherd : 
        public boost::enable_shared_from_this<Watcherd>  
    //     public boost::noncopyable, 
    {
        public:

            /** Create a new watcher daemon.
             * @param[in] ro set the writablity of the event database for the server
             */
            Watcherd(bool ro=false);

            virtual ~Watcherd(); 

            /** Beging listening for client connections.
             * @param[in] address the DNS hostname or IP address to listen on
             * @param[in] port the TCP port number or service name to listen on
             * @param[in] threadNum the number of threads to process connections in parallel
             */
            void run(const std::string &address, const std::string &port, const int &threadNum);

            libconfig::Config& config() { return config_; }

            /** determine if this watcher daemon was invoked in read-only event database mode. */
            bool readOnly() const { return readOnly_; }

	    /** send a list of the current streams to the specified client. */
	    void listStreams(ServerConnectionPtr);

	    /** Return a shared stream by UID. */
	    SharedStreamPtr getStream(uint32_t);

	    /** add a stream to the list of all known streams */
	    void addStream(SharedStreamPtr);

	    /** remove a stream from the list of all known streams */
	    void removeStream(SharedStreamPtr);

        private:

            DECLARE_LOGGER();

            ServerPtr serverConnection;
            boost::thread connectionThread;
            libconfig::Config &config_;

            ServerMessageHandlerPtr serverMessageHandlerPtr;

            bool readOnly_; // control whether or not the event db is writable

	    // List of *all* shared streams.
	    std::list<SharedStreamPtr> allStreams;
	    boost::shared_mutex allStreamsLock;
    };
}

#endif // WATCHERD_H

// vim:sw=4 ts=8
