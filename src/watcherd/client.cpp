#include "client.h"

using namespace watcher;
using namespace watcher::event;

INIT_LOGGER(Client, "Client");

Client::Client(
        const std::string& server_, 
        const std::string& service_) :
    ioService(),
    server(server_),
    service(service_),
    clientConnection(new ClientConnection(ioService, server, service))

{
    TRACE_ENTER();
    TRACE_EXIT();
}

Client::~Client()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

bool Client::sendMessage(const MessagePtr request)
{
    TRACE_ENTER(); 

    bool retVal=clientConnection->sendMessage(request);
    ioService.run();
    ioService.reset();

    TRACE_EXIT_RET((retVal ? "true" : "false"));
    return retVal;
}

bool Client::sendMessages(const std::vector<event::MessagePtr> &messages)
{
    TRACE_ENTER();

    bool retVal=clientConnection->sendMessages(messages);
    ioService.run();
    ioService.reset();

    TRACE_EXIT_RET((retVal ? "true" : "false"));
    return retVal;
}

void Client::setMessageHandler(MessageHandlerPtr messageHandler)
{
    TRACE_ENTER();
    clientConnection->setMessageHandler(messageHandler); 
    TRACE_EXIT();
}
