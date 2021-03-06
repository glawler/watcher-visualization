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
 * @file nodePropertiesMessage.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-09-04
 */
#ifndef WATCHER_NODEPROPS_MESSAGE_DATA_H
#define WATCHER_NODEPROPS_MESSAGE_DATA_H

#include <string>
#include <yaml-cpp/yaml.h>

#include "message.h"
#include "watcherColors.h"

namespace watcher {
    namespace event {
        /**
         * @class NodePropertiesMessage
         *
         * This class encapsulates a message describing the properties of a node. THis can be display
         * suggestions (shape, size, etc) or more abstract properties like clusterhead, attacker, victim,
         * etc. It is up to the GUIs to decide how to display the abstract properties and whether 
         * or not to honor the more concrete suggestions. 
         *
         * Command line executable for this message: @ref sendNodePropertiesMessage
         */
        class NodePropertiesMessage : public Message {
            public:

                /** @name node properties thacan be set/changed/modified */
                //@{
                /** Layer the the property is associated with. */
                GUILayer layer;

                /** The color of the node (this makes colorMessage redundant */
                Color color;

                /** Since there is no "NULL" color, set this to true if you want the 
                 * color in this message to be used.
                 */
                bool useColor;
                
                /** Suggested size of the node, deafult is -1==ignore. */
                float size;

                enum NodeShape { NOSHAPE=0, CIRCLE, SQUARE, TRIANGLE, TORUS, TEAPOT };
                static std::string nodeShapeToString(const NodeShape &shape);
                /** Convert a string into a shape. Sets argument to empty string if unsuccessful. */
                static bool stringToNodeShape(const std::string &s, NodeShape &shape);

                /** Suggested shape */
                NodeShape shape;

                /** if true, honor the shape variable */
                bool useShape; 

                enum DisplayEffect { NOEFFECT=0, SPARKLE, SPIN, FLASH };
                static std::string displayEffectToString(const NodePropertiesMessage::DisplayEffect &e); 
                /** Convert a string into an effect. Sets argument to empty string if unsuccessful. */
                static bool stringToDisplayEffect(const std::string &s, DisplayEffect &e); 
                typedef std::vector<DisplayEffect> DisplayEffectList;

                /** Suggested effects */
                DisplayEffectList displayEffects;

				// A string to use in place of the ip address as a label. 
				std::string label;

                /** Possible abstract properies of the node. NONE to unset. */
                enum NodeProperty { 
                    NOPROPERTY=0, 
                    LEAFNODE, 
                    NEIGHBORHOOD, 
                    REGIONAL, 
                    ROOT, 
                    ATTACKER, 
                    VICTIM, 
                    CHOSEN
                }; 

                static std::string nodePropertyToString(const NodeProperty &p);
                /** Convert a string into a property. Sets argument to empty string if unsuccessful. */
                static bool stringToNodeProperty(const std::string &s, NodeProperty &p);
                typedef std::vector<NodeProperty> NodePropertyList;

                /** Properties of the node */
                NodePropertyList nodeProperties;

                //@}

                /**
                 * Create a node properties messages.
                 */
                NodePropertiesMessage(); 

                /** 
                 * DEATH be not proud, though some have called thee
                 * Mighty and dreadfull, for, thou art not so,
                 * For, those, whom thou think'st, thou dost overthrow,
                 * Die not, poore death, nor yet canst thou kill me.
                 * From rest and sleepe, which but thy pictures bee,
                 * Much pleasure, then from thee, much more must flow,
                 * And soonest our best men with thee doe goe,
                 * Rest of their bones, and soules deliverie.
                 * Thou art slave to Fate, Chance, kings, and desperate men,
                 * And dost with poyson, warre, and sicknesse dwell,
                 * And poppie, or charmes can make us sleepe as well,
                 * And better then thy stroake; why swell'st thou then;
                 * One short sleepe past, wee wake eternally,
                 * And death shall be no more; death, thou shalt die.
                 */
                virtual ~NodePropertiesMessage();

                /** copy a the message 
                 * @param other the message to copy 
                 */
                NodePropertiesMessage(const NodePropertiesMessage &other);

                /** Compare this message against another.
                 * @param other the message to compare to
                 * @return bool, true if equal, false otherwise
                 */
                bool operator==(const NodePropertiesMessage &other) const;

                /** Set this message equal to another
                 * @param other the message to set this message equal to
                 * @return a reference to this instance
                 */
                NodePropertiesMessage &operator=(const NodePropertiesMessage &other);

                /** Write this message to <b>out</b> in human readable format 
                 * @param out the stream to write to
                 * @return the stream that was written to
                 */
                virtual std::ostream &toStream(std::ostream &out) const;

                /** Write this message to <b>out</b> in human readable format 
                 * @param out the stream to write to
                 * @return the stream that was written to
                 */
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
				virtual YAML::Node &serialize(YAML::Node &n); 
            private:
                DECLARE_LOGGER();
        };

        typedef boost::shared_ptr<NodePropertiesMessage> NodePropertiesMessagePtr; 

        std::ostream &operator<<(std::ostream &out, const NodePropertiesMessage &mess);
    }
}
#endif 
