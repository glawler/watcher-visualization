#ifndef MESSSAGE_HADNERLS_H
#define MESSSAGE_HADNERLS_H

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include "logger.h"
#include "message.h"

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

            // Handle a request and produce a reply.
            // 'request' is a copy of the message that was sent and produced the reply.
            // Returns true if the message should be sent, i.e. this request needs a reply.
            // false, otherwise.
            virtual bool produceReply(const MessagePtr &request, MessagePtr &reply) = 0;

            // Generate a message to send.
            virtual bool produceRequest(MessagePtr &request) = 0;

        private:

            DECLARE_LOGGER();
    };

    typedef boost::shared_ptr<MessageHandler> MessageHandlerPtr;

} // namespace 

#endif // MESSSAGE_HADNERLS_H
