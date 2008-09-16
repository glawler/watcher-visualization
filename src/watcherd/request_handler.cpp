#include "request_handler.hpp"
#include "messageStatus.h"

using namespace watcher;
using namespace boost;

INIT_LOGGER(watcher::request_handler, "request_handler");

request_handler::request_handler()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

void request_handler::handle_request(const boost::shared_ptr<Message> req, boost::shared_ptr<Message> &rep)
{
    TRACE_ENTER();

    // GTL TO DO: send an error message for now.
    // Do real work later. 
    rep=shared_ptr<MessageStatus>(new MessageStatus(MessageStatus::status_nack));

    LOG_DEBUG("request_handler sending back response: " << *rep);

    TRACE_EXIT();
}

