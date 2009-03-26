#ifndef WATCHER_LABEL_MESSAGE_DATA_H
#define WATCHER_LABEL_MESSAGE_DATA_H

#include <string>
#include <boost/asio.hpp>

#include "message.h"
#include "watcherColors.h"
#include "watcherGlobalFunctions.h"

namespace watcher {
    namespace event {
        class LabelMessage : public Message {
            public:
                // The data
                std::string label;
                int fontSize;
                Color foreground;
                Color background;
                unsigned int expiration;
                bool addLabel;              // add or remove the label depending on true or false here.

                // A label can be attached to a node or just hanging in space.
                // If address is defined it will be attached to a node, if
                // lat, lng, alt are defined is will hang in space. 
                boost::asio::ip::address address;
                float lat, lng, alt; 

                // Attach a label to this node.
                LabelMessage(const std::string &label="", const int fontSize=10); 

                // Attach a label to an arbitrary node.
                LabelMessage(const std::string &label, const boost::asio::ip::address &address, const int fontSize=10);

                // Float a label in space
                LabelMessage(const std::string &label, const float &lat, const float &lng, const float &alt, const int fontSize=10);

                LabelMessage(const LabelMessage &other);

                bool operator==(const LabelMessage &other) const;
                LabelMessage &operator=(const LabelMessage &other);

                virtual std::ostream &toStream(std::ostream &out) const;
                std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

            private:
                DECLARE_LOGGER();
        };

        typedef boost::shared_ptr<LabelMessage> LabelMessagePtr; 

        std::ostream &operator<<(std::ostream &out, const LabelMessage &mess);
    }
}

namespace boost {
    namespace serialization {
                template <typename Archive>
                void serialize(Archive & ar, watcher::event::LabelMessage &mess, const unsigned int file_version)
                {
                    TRACE_ENTER();
                    ar & boost::serialization::base_object<watcher::event::Message>(mess);
                    ar & mess.label;
                    ar & mess.foreground;
                    ar & mess.background;
                    ar & mess.expiration;
                    ar & mess.fontSize;
                    ar & mess.address;
                    ar & mess.addLabel;
                    ar & mess.lat;
                    ar & mess.lng;
                    ar & mess.alt;
                    TRACE_EXIT();
                }
    }
}

#endif // WATCHER_LABEL_MESSAGE_DATA_H
