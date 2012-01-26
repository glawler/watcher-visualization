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
 * @file nodeStatusMessage.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
/** 
 * @file
 * @author Geoff Lawler <geoff.lawler@sparta.com>
 * @date 2009-03-23
 */
#ifndef NODE_CONNECTION_MESSAGE_H
#define NODE_CONNECTION_MESSAGE_H

#include <string>
#include <yaml.h>

#include "message.h"
#include "watcherTypes.h"
#include "messageTypesAndVersions.h"

namespace watcher 
{
    namespace event 
    {
        /**
         * Notify GUI of node status  events.
         * @author Geoff Lawler <geoff.lawler@sparta.com>
         * @date 2009-03-23
         */
        class NodeStatusMessage : public Message 
        {
            public:
                enum statusEvent 
                {
                    connect,
                    disconnect
                };

                NodeStatusMessage(const statusEvent &event=connect); 

                NodeStatusMessage(const NodeStatusMessage &copy); 

                virtual ~NodeStatusMessage();

                bool operator==(const NodeStatusMessage &other) const;
                NodeStatusMessage &operator=(const NodeStatusMessage &other);

                virtual std::ostream &toStream(std::ostream &out) const;
                std::ostream &operator<<(std::ostream &out) const; 

                static std::string statusEventToString(const statusEvent &e); 

                statusEvent event;      // What happened

                GUILayer layer; 

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

        typedef boost::shared_ptr<NodeStatusMessage> NodeStatusMessagePtr; 
        std::ostream &operator<<(std::ostream &out, const NodeStatusMessage &mess);
    }
}
#endif


