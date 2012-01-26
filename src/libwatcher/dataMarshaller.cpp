/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Affero General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Affero General Public License for more details.
 * 
 *     You should have received a copy of the GNU Affero General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iomanip>
#include <sstream>
#include <yaml.h>

#include "dataMarshaller.h"
#include "logger.h"

INIT_LOGGER(watcher::DataMarshaller, "DataMarshaller"); 

using namespace std;
using namespace watcher;
using namespace watcher::event;

#define MARSHALSHORT(a,b) \
	do\
	{\
		*a++=((unsigned char)((b) >> 8)); \
		*a++=((unsigned char)(b)); \
	} while(0)

#define UNMARSHALSHORT(a,b)  {b=(*(a+0)<<8) | *(a+1); a+=2;}

#define MARSHALINT32(a,b) \
    do\
    {\
	*a++=((unsigned char)((b) >> 24));\
	*a++=((unsigned char)((b) >> 16));\
	*a++=((unsigned char)((b) >> 8)); \
	*a++=((unsigned char)(b)); \
    } while(0)

#define UNMARSHALINT32(a,b)  {b = (*(a+0)<<24) | (*(a+1)<<16) | (*(a+2)<<8) | *(a+3); a+=4;}

// static 
bool DataMarshaller::unmarshalHeader(const char *buffer, const size_t &bufferSize, size_t &payloadSize, unsigned short &messageNum)
{
    TRACE_ENTER();

    LOG_DEBUG("Unmarshalling a header of " << bufferSize << " bytes.");

    if (bufferSize < DataMarshaller::header_length)
    {
        LOG_ERROR("Not enought data in payload to unmarshal the packet or packet is corrupt"); 
        TRACE_EXIT_RET("false");
        return false;
    }

    if (bufferSize > 0xffffffff)
    {
	LOG_ERROR("buffer size " << bufferSize << " is too large to fit into an uint32.");
        TRACE_EXIT_RET("false");
        return false;
    }

    const unsigned char *bufPtr=(const unsigned char*)buffer; 
    UNMARSHALINT32(bufPtr, payloadSize);
    UNMARSHALSHORT(bufPtr, messageNum);

    LOG_DEBUG("Header data: payload size: " << payloadSize << " messageNum: " << messageNum); 

    return true;
}

//static 
bool DataMarshaller::unmarshalPayload(MessagePtr &message, const char *buffer, const size_t &bufferSize) {
	TRACE_ENTER(); 

	string data(buffer, bufferSize); 
	istringstream dataStream(data); 
    LOG_DEBUG("Unmarshalling payload data(size=" << bufferSize << "): " << data); 
	message=Message::unpack(dataStream); 
	if (!message) {
		LOG_WARN("Error: failed to unmarshal message."); 
		TRACE_EXIT_RET("false"); 
		return false; 
	}
	TRACE_EXIT_RET("true");
	return true; 
}

//static
bool DataMarshaller::unmarshalPayload(std::vector<MessagePtr> &messages, unsigned short &numOfMessages, const char *buffer, const size_t &bufferSize)
{
    TRACE_ENTER();

    unsigned short i=0;
	string str(buffer, bufferSize); 
	istringstream ss(str); 
	YAML::Parser parser(ss); 
	YAML::Node node; 
    for(i=0; i<numOfMessages; i++) {
		parser.GetNextDocument(node); 
		MessagePtr message=Message::unpack(node); 
		if (message.get()==0) { 
			numOfMessages=i; 
			TRACE_EXIT_RET("false"); 
			return false;
		}
		else {
			// marshalled with push_front, so should get same order on unmarshalling.
	        messages.push_back(message);      
		}
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

    uint32_t payloadSize=0;
    unsigned short messageNum=messages.size(); 

    // Putting each Message in a separate buffer may speed up sent/recv as
    // each buffer can be scatter-gather sent/recv'd.
	for(vector<MessagePtr>::const_iterator m=messages.begin(); m!=messages.end(); ++m) {
		// first pack the message so we know how big it is.
		ostringstream out; 
		m->get()->pack(out); 
		uint32_t size=static_cast<uint32_t>(out.str().size());
		payloadSize += size; 
		outBuffers.push_back(NetworkMarshalBuffer(out.str())); 
		LOG_DEBUG("Marshalled payload: " << out.str()); 
	}

    LOG_DEBUG("Serialized " << payloadSize << " bytes of message data from " << messageNum << " message" << (messageNum>1?"s":"")); 

    // Format the header.
	
    // GTL see the comment below on the YAML version of unmarshalHeader(). 
	//
	// // Format the header and put it on the front of the list.
	// YAML::Emitter emitter;
	// emitter << YAML::Flow << YAML::BeginMap;
	// emitter << YAML::Key << "payloadSize" << YAML::Value << payloadSize;
	// emitter << YAML::Key << "messageNum" << YAML::Value << messageNum;
	// emitter << YAML::EndMap; 
    // outBuffers.push_front(NetworkMarshalBuffer(string(emitter.c_str(), emitter.size()))); 
	
    // GTL - may be nice to put the header itself into a class that supports archive/serialization...
    unsigned char header[header_length];
    unsigned char *bufPtr=header; 
    MARSHALINT32(bufPtr, payloadSize);
    MARSHALSHORT(bufPtr, messageNum);
    outBuffers.push_front(NetworkMarshalBuffer(string((const char*)header, sizeof(header))));

    TRACE_EXIT_RET("true");
    return true;
}

// If we try to use YAML here, it breaks the async network reads as YAML has variable length
// encodings and the asio::read needs to know beforehand how many bytes to read for the header. 
// Code kept here in case I want to rewrite the network read code. Which seems very unlikely. 
//
// static 
// bool DataMarshaller::unmarshalHeader(const char *buffer, const size_t &bufferSize, size_t &payloadSize, unsigned short &messageNum)
// {
//     TRACE_ENTER();
//     LOG_DEBUG("Unmarshalling a header of " << bufferSize << " bytes.");
// 
//     if (bufferSize < DataMarshaller::headerLength())
//     {
//         LOG_ERROR("Not enought data in payload to unmarshal the packet or packet is corrupt"); 
//         TRACE_EXIT_RET("false");
//         return false;
//     }
// 
// 	try {
// 		istringstream in(string(buffer, bufferSize)); 
// 		YAML::Parser parser(in);
// 		YAML::Node node; 
// 		parser.GetNextDocument(node); 
// 		uint32_t tmp; 
// 		node["payloadSize"] >> tmp; 
// 		payloadSize=static_cast<size_t>(tmp); 
// 		node["messageNum"] >> messageNum; 
// 	}
// 	catch (YAML::ParserException &e) {
// 		LOG_ERROR("Error parsing message header: " << e.what()); 
// 		payloadSize=0; 
// 		messageNum=0; 
// 	}
// 
//     LOG_DEBUG("Header data: payload size: " << payloadSize << " messageNum: " << messageNum); 
// 
//     return true;
// }
// 

