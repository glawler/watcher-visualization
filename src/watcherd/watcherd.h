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

#include "libwatcher/message_fwd.h"

#include "watcherd_fwd.h"
#include "serverMessageHandler.h"
#include "server.h"
#include "libconfig.h++"
#include "logger.h"

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

            /** Subscribe a ServerConnection to the event stream. */
            void subscribe(ServerConnectionPtr);

            /** Unsubscribe a ServerConnection to the event stream. */
            void unsubscribe(ServerConnectionPtr);

            /** Post an event to all listening clients.
             * @param message message to send
             */
            void sendMessage(event::MessagePtr message);

            /** Post events to all listening clients.
             * @param messages messages to send
             */
            void sendMessage(const std::vector<event::MessagePtr>& messages);

            /// return a reference to the singleton config associated with this watcher daemon instance
            libconfig::Config& config() { return config_; }

            /// determine if this watcher daemon was invoked in read-only event database mode
            bool readOnly() const { return readOnly_; }

        protected:

        private:

            DECLARE_LOGGER();

            ServerPtr serverConnection;
            boost::thread connectionThread;
            libconfig::Config &config_;

            ServerMessageHandlerPtr serverMessageHandlerPtr;

            // List of clients subscribed to messages.
            typedef std::list<ServerConnectionPtr> MessageRequestors;
            MessageRequestors messageRequestors;
            pthread_rwlock_t messageRequestorsLock;

            bool readOnly_; // control whether or not the event db is writable
    };
}

#endif // WATCHERD_H
