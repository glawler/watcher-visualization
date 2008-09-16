#ifndef RESP_HANDLER_WATCHER_FOOBAR_YEEHAH_H
#define RESP_HANDLER_WATCHER_FOOBAR_YEEHAH_H

#include <string>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include "logger.h"
#include "message.h"

namespace watcher 
{
    /// The common handler for all incoming requests.
    class response_handler : private boost::noncopyable
    {
        public:
            /// Construct with a directory containing files to be served.
            explicit response_handler();

            /// Handle a request and produce a reply.
            void handle_response(const boost::shared_ptr<Message> request, boost::shared_ptr<Message> reply);

        private:
            DECLARE_LOGGER();

    };

} // namespace 

#endif // RESP_HANDLER_WATCHER_FOOBAR_YEEHAH_H
