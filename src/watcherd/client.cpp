#include "client.h"

using namespace watcher;
using namespace watcher::event;

INIT_LOGGER(Client, "Client");

Client::Client(
        const std::string& server_, 
        const std::string& service_) :
    ioService(),
    server(server_),
    service(service_)
{
    TRACE_ENTER();
    watcherdClientConnection = WatcherdClientConnectionPtr(new WatcherdClientConnection(ioService, server, service));
    TRACE_EXIT();
}

bool Client::sendMessage(const MessagePtr request)
{
    TRACE_ENTER(); 

    bool retVal=watcherdClientConnection->sendMessage(request);
    ioService.run();
    ioService.reset();

    TRACE_EXIT_RET((retVal ? "true" : "false"));
    return retVal;
}


