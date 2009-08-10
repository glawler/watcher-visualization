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
 * @file labelMessage.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
#ifndef WATCHER_LABEL_MESSAGE_DATA_H
#define WATCHER_LABEL_MESSAGE_DATA_H

#include <string>
#include <boost/asio.hpp>

#include "message.h"
#include "watcherColors.h"
#include "watcherGlobalFunctions.h"
#include "watcherTypes.h" 

namespace watcher {
    namespace event {
        /**
         * @class LabelMessage
         *
         * This class encapsulates a message containing a text label for a node. This message is sent to a watcherd instance. The GUI(s) 
         * attached to that watcherd instance then draw a label with the given text and attributes on the node.
         *
         * Command line executable for this message: @ref sendLabelMessage
         */
        class LabelMessage : public Message {
            public:

                /** The text of the label */
                std::string label;

                /** The fontsize of the text drawn */
                float fontSize;

                /** The forground color of the label */
                Color foreground;

                /** The background color of the label */
                Color background;

                /** How long the label lasts in milliseconds */
                Timestamp  expiration;   

                /** Whether or not to add or remove a label */
                bool addLabel;              // add or remove the label depending on true or false here.

                /** Layer the label should be on */
                GUILayer layer;             // which layer to put the label on 

                /** The coordinates of the label, if free floating */
                float lat, lng, alt; 

                /**
                 * Attach a label to this node.
                 * @param label the text to display on the label.
                 * @param fontSize the size of the font, if 0 use the GUI default size.
                 */
                LabelMessage(const std::string &label="", const int fontSize=0); 

                /**
                 * Attach a label to an arbitrary node.
                 * @param label the text to display on the label.
                 * @param address the ID of the node to attach the label to.
                 * @param fontSize the size of the font, if 0 use the GUI default size.
                 */
                LabelMessage(const std::string &label, const NodeIdentifier &address, const int fontSize=0);

                /**
                 * Float a label in space. 
                 * @param label the text to display on the label.
                 * @param lat the latitude of the label
                 * @param lng the longitude of the label
                 * @param alt the altitude of the label
                 * @param fontSize the size of the font, if 0 use the GUI default size.
                 */
                LabelMessage(const std::string &label, const float &lat, const float &lng, const float &alt, const int fontSize=0);

                /** copy a the message 
                 * @param other the message to copy 
                 */
                LabelMessage(const LabelMessage &other);

                /** Compare this message against another.
                 * @param other the message to compare to
                 * @return bool, true if equal, false otherwise
                 */
                bool operator==(const LabelMessage &other) const;

                /** Set this message equal to another
                 * @param other the message to set this message equal to
                 * @return a reference to this instance
                 */
                LabelMessage &operator=(const LabelMessage &other);

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

            private:
                friend class boost::serialization::access;
                template <typename Archive> void serialize(Archive & ar, const unsigned int file_version);
                DECLARE_LOGGER();
        };

        typedef boost::shared_ptr<LabelMessage> LabelMessagePtr; 

        std::ostream &operator<<(std::ostream &out, const LabelMessage &mess);
    }
}
#endif // WATCHER_LABEL_MESSAGE_DATA_H
