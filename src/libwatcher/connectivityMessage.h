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
 * @file connectivityMessage.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
#ifndef CONNNECTIVITY_MESSAGE_DATA_H
#define CONNNECTIVITY_MESSAGE_DATA_H

#include <vector>

#include "message.h"
#include "watcherTypes.h"

namespace watcher 
{

    namespace event 
    {
        /**
         * Inform the GUIs of the connectivity (neighbors)
         * this node has. It's up to the GUI implementation
         * to figure out how to display the connections.
         * 
         * @author Geoff Lawler <Geoff.Lawler@cobham.com>
         * @date 2009-05-06
         *
         * Command line executable for this message: @ref sendConnectivityMessage
         */
        class ConnectivityMessage : public Message
        {
            public:
                
                /**
                 * List of neighbors at the time the message is sent.
                 */
                typedef std::vector<watcher::NodeIdentifier> NeighborList;
                NeighborList neighbors;

                /**
                 * Which layer this is on.
                 */
                GUILayer layer;

                ConnectivityMessage();
                virtual ~ConnectivityMessage(); 

                ConnectivityMessage(const ConnectivityMessage &other);

                bool operator==(const ConnectivityMessage &other) const;
                ConnectivityMessage &operator=(const ConnectivityMessage &other);

                virtual std::ostream &toStream(std::ostream &out) const;
                std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

				/** Serialize this message using a YAML::Emitter
				 * @param e the emitter to serialize to
				 * @return the emitter emitted to.
				 */
				virtual YAML::Emitter &serialize(YAML::Emitter &e) const; 

				/** Serialize from a YAML::Parser. 
				 * @param p the Parser to read from 
				 * @return the parser read from. 
				 */
				virtual YAML::Node &serialize(YAML::Node &node); 
            private:
                DECLARE_LOGGER();
        };

        typedef boost::shared_ptr<ConnectivityMessage> ConnectivityMessagePtr; 

        std::ostream &operator<<(std::ostream &out, const ConnectivityMessage &mess);
    }
}

#endif // CONNNECTIVITY_MESSAGE_DATA_H
