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
 * @file edgeMessage.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
#ifndef WATCHER_EDGE_MESSAGE_DATA_H
#define WATCHER_EDGE_MESSAGE_DATA_H

#include <string>
#include <yaml-cpp/yaml.h>

#include "message.h"
#include "labelMessage.h"
#include "watcherColors.h"
#include "colors.h"
#include "watcherTypes.h" 

namespace watcher {
    namespace event {
        /// annotate a connection between two nodes
        class EdgeMessage : public Message {
            public:
                // The data
                NodeIdentifier node1;
                NodeIdentifier node2;
                Color edgeColor;
                Timestamp expiration; 
                float width;      
                GUILayer layer;
                bool addEdge;   // if true, add an edge, else remove existing edge;
                LabelMessagePtr middleLabel;
                LabelMessagePtr node1Label;
                LabelMessagePtr node2Label;
                bool bidirectional;

                EdgeMessage();
                EdgeMessage(
                            const NodeIdentifier &node1_, // address of node1
                            const NodeIdentifier &node2_, // address of node2
                            const GUILayer &layer_=PHYSICAL_LAYER,  // Which GUI layer the edge is on
                            const Color &c_=colors::blue,            // color of edge
                            const float &width=2.0,                 // width of the edge 
                            const bool bidirectional_=false,        // Is this edge bidirectional?
                            const Timestamp  expiration_=Infinity,  // expiration time in milliseconds, 0=never expire
                            const bool &addEdge=true);              // If true, add an edge, else remove exising edge with same props as this one.

                EdgeMessage(const EdgeMessage &other);

                void setMiddleLabel(const LabelMessagePtr &label); 
                void setNode1Label(const LabelMessagePtr &label); 
                void setNode2Label(const LabelMessagePtr &label); 

                bool operator==(const EdgeMessage &other) const;
                EdgeMessage &operator=(const EdgeMessage &other);

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

        typedef boost::shared_ptr<EdgeMessage> EdgeMessagePtr; 

        std::ostream &operator<<(std::ostream &out, const EdgeMessage &obj);
    }
}
#endif // WATCHER_EDGE_MESSAGE_DATA_H
