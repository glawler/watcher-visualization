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
        class LabelMessage : public Message {
            public:
                // The data
                std::string label;
                float fontSize;
                Color foreground;
                Color background;
                Timestamp  expiration;    // this should be a float
                bool addLabel;              // add or remove the label depending on true or false here.
                GUILayer layer;             // which layer to put the label on 

                float lat, lng, alt; 

                /**
                 * Attach a label to this node.
                 * @param label - the text to display on the label.
                 * @param fontSize - the size of the font, if 0 use the GUI default size.
                 */
                LabelMessage(const std::string &label="", const int fontSize=0); 

                /**
                 * Attach a label to an arbitrary node.
                 * @param label - the text to display on the label.
                 * @param nodeId - the ID of the node to attach the label to.
                 * @param fontSize - the size of the font, if 0 use the GUI default size.
                 */
                LabelMessage(const std::string &label, const NodeIdentifier &address, const int fontSize=0);

                /**
                 * Float a label in space. 
                 * @param label - the text to display on the label.
                 * @param lat - the latitude of the label
                 * @param long - the longitude of the label
                 * @param alt - the altitude of the label
                 * @param fontSize - the size of the font, if 0 use the GUI default size.
                 */
                LabelMessage(const std::string &label, const float &lat, const float &lng, const float &alt, const int fontSize=0);

                LabelMessage(const LabelMessage &other);

                bool operator==(const LabelMessage &other) const;
                LabelMessage &operator=(const LabelMessage &other);

                virtual std::ostream &toStream(std::ostream &out) const;
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
