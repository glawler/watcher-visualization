/**
 * @file colorMessage.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
#ifndef WATCHER_COLOR__MESSAGE_DATA_H
#define WATCHER_COLOR__MESSAGE_DATA_H

#include <string>
#include <boost/asio.hpp>

#include "message.h"
#include "watcherColors.h"
#include "watcherTypes.h" 

namespace watcher {
    namespace event {
        /**
         * This class encapsulates a message containing color information for a node. When a GUI attached to a watcherd instance 
         * gets this message it colors the node appropriately. The message also supports an expiration time (only color for a 
         * given number of milliseconds) and flashing the node different colors (for an optional length of time as well). 
         *
         * @author Geoff Lawler <geoff.lawler@cobham.com>
         * @date 2009-07-15
         * @class ColorMessage
         * Command line executable for this message: @ref sendColorMessage
         */
        class ColorMessage : public Message {
            public:

                /** The color to make the node */
                Color color;

                /** The rate at which the node will flash in milliseconds */
                Timestamp flashPeriod; 

                /** How long to modify the node's color in milliseconds */
                Timestamp expiration;  

                /* The layer that the color effects */
                GUILayer layer; 

                /** Create a ColorMessage with default values black, 127.0.0.1, no expire, no flash */
                ColorMessage();  // default: black, 127.0.0.1, no expire, no flash
                /** Create a ColorMessage
                 * @param c, the color to make the node
                 * @param address, the ip address of the node - localhost if not given
                 * @param expiration, How long to modify the node's color in milliseconds, forever if not given
                 * @param flashPeriod, The rate at which the node will flash in milliseconds
                 */
                ColorMessage(
                             const Color &c, 
                             const boost::asio::ip::address &address=boost::asio::ip::address::from_string("127.0.0.1"),
                             const Timestamp &expiration=0,
                             const Timestamp &flashPeriod=0);

                /** copy a ColorMessage 
                 * @param other, the message to copy 
                 */
                ColorMessage(const ColorMessage &other);

                /** Compare this message against another.
                 * @param other, the message to compare to
                 * @returns bool, true if equal, false otherwise
                 */
                bool operator==(const ColorMessage &other) const;

                /** Set this message equal to another
                 * @param other, the message to set this message equal to
                 * @ret ColorMessage, a reference to this instance
                 */
                ColorMessage &operator=(const ColorMessage &other);

                /** Write this message to <b>out</b> in human readable format 
                 * @param out, the stream to write to
                 * @return the stream that was written to
                 */
                virtual std::ostream &toStream(std::ostream &out) const;

                /** Write this message to <b>out</b> in human readable format 
                 * @param out, the stream to write to
                 * @return the stream that was written to
                 */
                std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

            private:
                friend class boost::serialization::access;
                template <typename Archive> void serialize(Archive& ar, const unsigned int file_version);
                DECLARE_LOGGER();
        };

        typedef boost::shared_ptr<ColorMessage> ColorMessagePtr; 

        std::ostream &operator<<(std::ostream &out, const ColorMessage &mess);
    }
}
#endif // WATCHER_COLOR__MESSAGE_DATA_H
