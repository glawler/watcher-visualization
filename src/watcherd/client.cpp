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
    TRACE_EXIT();
}

bool Client::sendMessage(const boost::shared_ptr<Message> request)
{
    TRACE_ENTER(); 

    clientConnection = ClientConnectionPtr(new ClientConnection(ioService, server, service));
    bool retVal=clientConnection->sendMessage(request);
    ioService.run();

    TRACE_EXIT_RET((retVal ? "true" : "false"));
    return retVal;
}


