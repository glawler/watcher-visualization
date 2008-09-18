#include "client.hpp"

using namespace watcher;

INIT_LOGGER(Client, "Client");

Client::Client(
        const std::string& server_, 
        const std::string& service_) :
    ioService(),
    server(server_),
    service(service_)
{
    TRACE_ENTER();
    clientConnection = clientConnectionPtr(new ClientConnection(ioService, server, service));
    ioService.run();
    TRACE_EXIT();
}

bool Client::sendMessage(const boost::shared_ptr<Message> request)
{
    TRACE_ENTER(); 
    bool retVal=clientConnection->sendMessage(request);
    TRACE_EXIT_RET((retVal ? "true" : "false"));
    return retVal;
}


