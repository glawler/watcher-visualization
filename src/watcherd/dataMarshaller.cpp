#include <iomanip>
#include <sstream>

#include <boost/lexical_cast.hpp>

#include "dataMarshaller.h"
#include "logger.h"

INIT_LOGGER(watcher::DataMarshaller, "DataMarshaller"); 

using namespace std;
using namespace watcher;
using namespace watcher::event;

// static 
bool DataMarshaller::unmarshalHeader(const char *buffer, const size_t &bufferSize, size_t &payloadSize, unsigned short &messageNum)
{
    TRACE_ENTER();
    payloadSize=0;

    // logger is crashing if given anything other than a string! GTL - Test and see if this is still true.
    LOG_DEBUG("Unmarshalling a header of " << boost::lexical_cast<std::string>(bufferSize) << " bytes.");

    if (bufferSize < DataMarshaller::header_length)
    {
        LOG_ERROR("Not enought data in payload to unmarshal the packet or packet is corrupt"); 
        TRACE_EXIT_RET("false");
        return false;
    }

    string inbound_header(buffer, buffer + header_length);
    stringstream is(inbound_header);
    if (!(is >> std::setw(sizeof(std::string::size_type)) >> std::hex >> payloadSize) || 
        !(is >> std::setw(sizeof(messageNum)) >> std::hex >> messageNum))
    {
        // Header doesn't seem to be valid. Inform the caller.
        LOG_DEBUG("Error reading payloadsize and type from header: "  << strerror(errno)); 
        TRACE_EXIT_RET("false");
        return false;
    }

    return true;
}

//static 
bool DataMarshaller::unmarshalPayload(MessagePtr &message, const char *buffer, const size_t &bufferSize)
{
    TRACE_ENTER(); 

    // Extract the Message from the payload
    istringstream s(string(buffer, bufferSize));
    message=Message::unpack(s);

    TRACE_EXIT_RET((message?"true":"false"));
    return static_cast<bool>(message);          // cast may be redundant 
}

//static
bool DataMarshaller::unmarshalPayload(std::vector<MessagePtr> &messages, unsigned short &numOfMessages, const char *buffer, const size_t &bufferSize)
{
    TRACE_ENTER();

    istringstream is(string(buffer, bufferSize)); 
    unsigned short i=0;
    for(i=0; i<numOfMessages; i++)
    {
        MessagePtr m=Message::unpack(is);
        if(!m)
        {
            LOG_WARN("Error: Unable to unmarshal message number " << i+1 << " in the packet payload."); 
            numOfMessages=i;
            TRACE_EXIT_RET("false"); 
            return false;
        }
        messages.push_back(m);      // marshalled with push_front, so should get same order on unmarshalling.
    }

    LOG_DEBUG("Successfully unmarshalled " << i << " message" << (i>0?"s":"")); 

    TRACE_EXIT_RET("true"); 
    return true;
}

//static 
bool DataMarshaller::marshalPayload(const MessagePtr &message, NetworkMarshalBuffers &outBuffers)
{ 
    TRACE_ENTER();

    std::vector<MessagePtr> messVec;
    messVec.push_back(message);
    bool retVal=marshalPayload(messVec, outBuffers);

    TRACE_EXIT_RET((retVal?"true":"false")); 
    return retVal;
}

//static 
bool DataMarshaller::marshalPayload(const vector<MessagePtr> &messages, NetworkMarshalBuffers &outBuffers)
{
    TRACE_ENTER();

    // Add serialized Messages first, then prepend the header once we know 
    // how large the payload will be.
    unsigned short payloadSize=0;
    unsigned short messageNum=messages.size(); 
    
    // Putting each Message in a separate buffer may speed up sent/recv as
    // each buffer can be scatter-gather sent/recv'd.
    ostringstream os;
    for(vector<MessagePtr>::const_iterator m=messages.begin(); m!=messages.end(); ++m)
    {
        (*m)->pack(os); 
        payloadSize+=os.str().size(); 
        outBuffers.push_front(NetworkMarshalBuffer(os.str().c_str(), os.str().size())); 
        os.str(""); 
    }

    LOG_DEBUG("Serialized " << payloadSize << " bytes of message data"); 

    // Format the header.
    // GTL - may be nice to put the header itself into a class that supports archive/serialization...
    os << std::setw(sizeof(payloadSize)) << std::hex << payloadSize;
    os << std::setw(sizeof(messageNum))  << std::hex << messageNum; 
    if (!os || os.str().size() != header_length)
    {
        LOG_DEBUG("Error serializing the message header"); 
        TRACE_EXIT_RET("false");
        return false;
    }

    // Put the header on the front.
    outBuffers.push_front(NetworkMarshalBuffer(os.str().c_str(), os.str().size()));

    TRACE_EXIT_RET("true");
    return true;
}


