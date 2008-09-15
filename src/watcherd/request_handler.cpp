#include "request_handler.hpp"

using namespace watcher;
using namespace server;

INIT_LOGGER(watcher::server::request_handler, "request_handler");

request_handler::request_handler()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

void request_handler::handle_request(const boost::shared_ptr<Message> req, boost::shared_ptr<Message> rep)
{
    TRACE_ENTER();

    // GTL TO DO: send an ack for now.
    // Do real work later. 

    TRACE_EXIT();
}

