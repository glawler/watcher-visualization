#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/list.hpp>

#include "dataRequestMessage.h"
#include "messageTypesAndVersions.h"

using namespace std;
using namespace watcher;

INIT_LOGGER(DataRequestMessage, "Message.DataRequestMessage");
BOOST_CLASS_EXPORT_GUID(DataRequestMessage, "DataRequestMessage"); 

DataRequestMessage::DataRequestMessage() : 
    Message(DATA_REQUEST_MESSAGE_TYPE, DATA_REQUEST_MESSAGE_VERSION),
    startingAt(0),
    timeFactor(1),
    layers(0)
{
    TRACE_ENTER();
    TRACE_EXIT();
}
DataRequestMessage::DataRequestMessage(
        const MessageTypeList &types, 
        const Timestamp &startAt_,
        const float &factor_,
        const unsigned int layers_) : 
    Message(DATA_REQUEST_MESSAGE_TYPE, DATA_REQUEST_MESSAGE_VERSION),
    dataTypesRequested(types),
    startingAt(startAt_),
    timeFactor(factor_),
    layers(layers_)
{
    TRACE_ENTER();
    TRACE_EXIT();
}

DataRequestMessage::DataRequestMessage(const DataRequestMessage &other)
{
    TRACE_ENTER();
    (*this)=other;
    TRACE_EXIT();
}

DataRequestMessage::~DataRequestMessage()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

void DataRequestMessage::requestAllMessages()
{
    MessageType types[] = {  
        MESSAGE_STATUS_TYPE,
        TEST_MESSAGE_TYPE,
        GPS_MESSAGE_TYPE, 
        LABEL_MESSAGE_TYPE, 
        EDGE_MESSAGE_TYPE,
        COLOR_MESSAGE_TYPE,
        DATA_REQUEST_MESSAGE_TYPE
    };

    GUILayer theLayers[] = { 
        HIERARCHY_LAYER,
        ROUTING_LAYER,
        NODE_LAYER,
        FLOATING_LAYER,
    };

   for(unsigned int i = 0; i < sizeof(types)/sizeof(types[0]); i++)
       dataTypesRequested.push_back(types[i]);

   for(unsigned int i = 0; i < sizeof(theLayers)/sizeof(theLayers[0]); i++)
        layers |= theLayers[i];
}

bool DataRequestMessage::operator==(const DataRequestMessage &other) const
{
    TRACE_ENTER();
    
    bool retVal = 
        Message::operator==(other) && 
        dataTypesRequested==other.dataTypesRequested &&
        startingAt==other.startingAt && 
        timeFactor==other.timeFactor &&
        layers==other.layers;

    TRACE_EXIT_RET(retVal);
    return retVal;
}

DataRequestMessage &DataRequestMessage::operator=(const DataRequestMessage &other)
{
    TRACE_ENTER();

    Message::operator=(other);
    dataTypesRequested=other.dataTypesRequested;
    startingAt=other.startingAt;
    timeFactor=other.timeFactor;
    layers=other.layers;

    TRACE_EXIT();
    return *this;
}

// virtual
std::ostream &DataRequestMessage::toStream(std::ostream &out) const
{
    TRACE_ENTER();
    Message::toStream(out); 
    out << " types: "; 
    MessageTypeList::const_iterator t;
    for (t=dataTypesRequested.begin(); t!=dataTypesRequested.end(); t++)
        out << *t << " ";
    out << " starting at: " << startingAt; 
    out << " time factor: " << timeFactor;
    out << " layers: " << layers;
    TRACE_EXIT();
    return out;
}

ostream &watcher::operator<<(ostream &out, const DataRequestMessage &mess)
{
    TRACE_ENTER();
    mess.operator<<(out);
    TRACE_EXIT();
    return out;
}

void DataRequestMessage::serialize(boost::archive::polymorphic_iarchive & ar, const unsigned int file_version)
{
    TRACE_ENTER();
    ar & boost::serialization::base_object<Message>(*this);
    ar & dataTypesRequested;
    ar & startingAt;
    ar & timeFactor;
    ar & layers;
    TRACE_EXIT();
}

void DataRequestMessage::serialize(boost::archive::polymorphic_oarchive & ar, const unsigned int file_version)
{
    TRACE_ENTER();
    ar & boost::serialization::base_object<Message>(*this);
    ar & dataTypesRequested;
    ar & startingAt;
    ar & timeFactor;
    ar & layers;
    TRACE_EXIT();
}

