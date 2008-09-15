#ifndef REQ_HANDLER_WATCHER_FOOBAR_YEEHAH_H
#define REQ_HANDLER_WATCHER_FOOBAR_YEEHAH_H

#include <string>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include "logger.h"
#include "message.h"

namespace watcher 
{
    namespace server 
    {
        /// The common handler for all incoming requests.
        class request_handler : private boost::noncopyable
        {
            public:
                /// Construct with a directory containing files to be served.
                explicit request_handler();

                /// Handle a request and produce a reply.
                void handle_request(const boost::shared_ptr<Message> req, boost::shared_ptr<Message> rep);

                DECLARE_LOGGER();

            private:
        };

    } // namespace server
} // namespace 

#endif // REQ_HANDLER_WATCHER_FOOBAR_YEEHAH_H
