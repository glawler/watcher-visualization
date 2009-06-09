#ifndef MESSSAGE_HADNERLS_H
#define MESSSAGE_HADNERLS_H

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

#include "libwatcher/message.h"
#include "libwatcher/connection_fwd.h"

namespace watcher 
{
    class MessageHandler : public boost::noncopyable
    {
        public:
            /// Construct with a directory containing files to be served.
            MessageHandler();
            virtual ~MessageHandler(); 

            /**
             * Notification of message arrival. Default does nothing, but log the message. 
             * Classes which derive from this class should handle the message arrival and if
             * they want to respond to the message, make 'response' non-null on return.
             * @param[in] - message - the newly arrived message. 
             * @return boolean. if false, keep connection open, true otherwise.
             */
            virtual bool handleMessageArrive(ConnectionPtr, const event::MessagePtr &message);
            virtual bool handleMessagesArrive(ConnectionPtr, const std::vector<event::MessagePtr> &messages);

            /**
             * Notification that a message has been successfully sent.
             *
             * @param[in] - the message that was sent
             * @return - boolean. If true, shutdown the write half of the socket, flase otherwise
             */
            virtual bool handleMessageSent(const event::MessagePtr &message); 
            virtual bool handleMessagesSent(const std::vector<event::MessagePtr> &messages);

        private:

            DECLARE_LOGGER();
    };

    typedef boost::shared_ptr<MessageHandler> MessageHandlerPtr;

} // namespace 

#endif // MESSSAGE_HADNERLS_H
