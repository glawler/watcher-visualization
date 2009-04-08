#ifndef MESSSAGE_HADNERLS_H
#define MESSSAGE_HADNERLS_H

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

#include "libwatcher/message.h"

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
             * @param[out] - response. If non-null, this message will be sent back to the entity
             * which sent 'message' as a response.
             * @return - boolean. true, if message handled, false on error or otherwise. If the return
             * value is negative, the response message will not be sent even if non-null.
             */
            virtual bool handleMessageArrive(const event::MessagePtr &message, event::MessagePtr &response);
            virtual bool handleMessagesArrive(const std::vector<event::MessagePtr> &messages, event::MessagePtr &response);

        private:

            DECLARE_LOGGER();
    };

    typedef boost::shared_ptr<MessageHandler> MessageHandlerPtr;

} // namespace 

#endif // MESSSAGE_HADNERLS_H
