#include "messageHandlerFactory.h"
#include "messageStatusHandler.h"
#include "testMessageHandler.h"
#include "gpsMessageHandler.h"
#include "labelMessageHandler.h"
#include "edgeMessageHandler.h"
#include "colorMessageHandler.h"
#include "dataRequestMessageHandler.h"

using namespace watcher;
using namespace watcher::event;

INIT_LOGGER(MessageHandlerFactory, "MessageHandlerFactory");

//
// If the messagehandlers stay just as a couple of fucntions, I may make them 
// all singletons and return pointers to static instances in order to save on
// memory and 'new'ing time. 
//
// static 
boost::shared_ptr<MessageHandler> MessageHandlerFactory::getMessageHandler(const MessageType &type)
{
    switch(type)
    {
        case UNKNOWN_MESSAGE_TYPE: 
            boost::shared_ptr<MessageHandler>(); // == NULL
            break;
        case MESSAGE_STATUS_TYPE: 
            return boost::shared_ptr<MessageStatusHandler>(new MessageStatusHandler);
            break;
        case TEST_MESSAGE_TYPE: 
            return boost::shared_ptr<TestMessageHandler>(new TestMessageHandler);
            break;
        case GPS_MESSAGE_TYPE: 
            return boost::shared_ptr<GPSMessageHandler>(new GPSMessageHandler);
            break;
        case LABEL_MESSAGE_TYPE: 
            return boost::shared_ptr<LabelMessageHandler>(new LabelMessageHandler);
            break;
        case EDGE_MESSAGE_TYPE: 
            return boost::shared_ptr<EdgeMessageHandler>(new EdgeMessageHandler);
            break;
        case COLOR_MESSAGE_TYPE: 
            return boost::shared_ptr<ColorMessageHandler>(new ColorMessageHandler);
            break;
        case DATA_REQUEST_MESSAGE_TYPE: 
            return boost::shared_ptr<DataRequestMessageHandler>(new DataRequestMessageHandler);
            break;

            // GTL - don't put default case
    }

    return boost::shared_ptr<MessageHandler>(); // == NULL
}
