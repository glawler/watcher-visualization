#ifndef MESSSAGE_HADNERLS_H
#define MESSSAGE_HADNERLS_H

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <libwatcher/message.h>

namespace watcher 
{
    /// Base class for handling message sending and receiving.
    //
    class MessageHandler : public boost::noncopyable
    {
        public:
            /// Construct with a directory containing files to be served.
            MessageHandler();
            virtual ~MessageHandler(); 

            typedef enum 
            {
                readMessage,        // Read another message immediately
                writeMessage,       // Write another message immediately
                closeConnection,    // Close the connection.
                stayConnected       // Just stay connected and wait for new things to happen
            } ConnectionCommand;

            // Handle a request and produce a reply.
            // Default is to produce a MessageStatus with status of status_ack
            virtual ConnectionCommand produceReply(const event::MessagePtr &request, event::MessagePtr &reply);

            // Handle a reply to a request. Default is to check for a MessageStatus with status of 
            // status_ok or status_ack.
            virtual ConnectionCommand handleReply(const event::MessagePtr &request, const event::MessagePtr &reply);

            // Generate a message to send, pure virtual. 
            virtual ConnectionCommand produceRequest(event::MessagePtr &request) = 0;

            // Notification of message arrival. Default does nothing, but log the message. 
            virtual void handleMessageArrive(const event::MessagePtr message); 

        private:

            DECLARE_LOGGER();
    };

    typedef boost::shared_ptr<MessageHandler> MessageHandlerPtr;

} // namespace 

#endif // MESSSAGE_HADNERLS_H
