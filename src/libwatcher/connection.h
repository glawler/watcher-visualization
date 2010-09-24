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
 * @file connection.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
#ifndef WATHER_CONNECTION_H
#define WATHER_CONNECTION_H

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <list>

#include "messageHandlerFwd.h"
#include "connection_fwd.h"

namespace watcher
{
    /// Base class for common functionality among different types of connections to the watcher daemon.
    class Connection :
        private boost::noncopyable
    {
        public:
            /** Create a new connection.
             * @param io_service the ASIO service for the application
             */
            Connection(boost::asio::io_service& io_service);
            virtual ~Connection();

            /// convenience typedef to refer to the socket type for the connection
            typedef boost::asio::ip::tcp::socket ConnectionSocket;

            /** Retrieve the ASIO socket object associated with this connection.
             * @return reference to socket object
             */
            ConnectionSocket &getSocket();

            /**
             * Set a messageHandler if you want direct access to the 
             * responses sent via sendMessage().
             * @param messageHandler an instance of a message handler class. 
             */
            void addMessageHandler(MessageHandlerPtr messageHandler); 

            /**
             * Removes a message handler set via the addMessageHandler member function.
             * @param messageHandler an instance of a message handler class. 
             */
            void removeMessageHandler(MessageHandlerPtr messageHandler); 

            /**
             * Returns the address of this connection's peer.
             */
            const boost::asio::ip::tcp::endpoint remoteEndpoint() const { return theSocket.remote_endpoint(); }

	    /**
	     * Returns the part of the 'hostsvc' string preceding the ':' 
	     * (or the whole hostsvc if it contains no ':').
	     * If the hostname part is empty, returns localhost.
	     *
	     * @param hostsvc a [<host>][:<service>] string
	     */
	    static std::string getServerHost(const std::string& hostsvc);

	    /**
	     * Returns the service (a name or port string) to use, 
	     * given a hostsvc and optional explicit service.
	     * If no service is given in either argument, returns
	     * the default service name "watcherd".
	     *
	     * @param hostsvc a [<host>][:<service>] string
	     * @param service if nonempty, this overrides service
	     */
	    static std::string getServerService(const std::string& hostsvc, const std::string& service);

        protected:

            typedef std::list<MessageHandlerPtr> MessageHandlerList;
            MessageHandlerList messageHandlers; 

            ConnectionSocket theSocket;

            std::string endpoint_addr_; //< IP address of the connection endpoint
            unsigned short endpoint_port_; //< TCP/IP port of the connection endpoint

        private:

            DECLARE_LOGGER(); 
    };

}


#endif // WATHER_CONNECTION_H
