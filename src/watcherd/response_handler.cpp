#include "response_handler.hpp"
#include "messageStatus.h"

using namespace watcher;
using namespace boost;
using namespace std;

INIT_LOGGER(response_handler, "response_handler");

response_handler::response_handler()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

void response_handler::handle_response(const boost::shared_ptr<Message> request, boost::shared_ptr<Message> reply)
{
    TRACE_ENTER();

    LOG_DEBUG("Got response from request.");
    LOG_DEBUG("     Request: " << request);
    LOG_DEBUG("     Reply: " << reply);

    switch(reply->type)
    {
        case MESSAGE_STATUS_TYPE:
            {
                LOG_DEBUG("Recv'd a status message: " << reply);
                break;
            }
        default:
            LOG_DEBUG("Recv'd non status reply from message: " << reply);
    }

    TRACE_EXIT();
}

