#ifndef THERES_A_NEW_DATA_MARHSAL_IN_TOWN_H
#define THERES_A_NEW_DATA_MARHSAL_IN_TOWN_H

#include <string>
#include <deque>

#include <boost/asio.hpp>

#include "libwatcher/message.h"

namespace watcher 
{
    // A reference-counted non-modifiable buffer class.
    class shared_const_buffer
    {
        public:
            // Construct from a std::string.
            explicit shared_const_buffer(const std::string& data)
                : data_(new std::vector<char>(data.begin(), data.end())),
                buffer_(boost::asio::buffer(*data_))
        {
        }

            // Implement the ConstBufferSequence requirements.
            typedef boost::asio::const_buffer value_type;
            typedef const boost::asio::const_buffer* const_iterator;
            const boost::asio::const_buffer* begin() const { return &buffer_; }
            const boost::asio::const_buffer* end() const { return &buffer_ + 1; }

            operator boost::asio::const_buffer() const { return buffer_; } 

        private:
            boost::shared_ptr<std::vector<char> > data_;
            boost::asio::const_buffer buffer_;
    };
    
    /**
     * The DataMarshaller class marshals and unmarshals Messages for transport
     * over the network. It appends a small header (size, type) to the self-serializing
     * watcher::event::Messages. 
     *
     * DataMarshaller is a static class - it just contains static methods. 
     */
    class DataMarshaller 
    {
        public:

            /** 
             * @typedef a sequence of buffers used to write to the network.
             */
            typedef shared_const_buffer NetworkMarshalBuffer;
            // typedef std::vector<NetworkMarshalBuffer> NetworkMarshalBuffers;
            typedef std::deque<NetworkMarshalBuffer> NetworkMarshalBuffers;
            typedef boost::shared_ptr<NetworkMarshalBuffers> NetworkMarshalBuffersPtr;

            /**
             * Unmarshal a header, returning the length of the payload which follows. 
             * @param[in] - buffer, the buffer which contains the header data
             * @param[in] - bufferSize, the size of 'buffer'
             * @param[out] - payloadSize, the size of the payload that the header is attached to.
             * @param[out] - messageNum, the number of messages in the payload.
             * @return - true on success, false otherwise.
             */
            static bool unmarshalHeader(
                    const char *buffer, 
                    const size_t &bufferSize, 
                    size_t &payloadSize, 
                    unsigned short &messageNum);

            /**
             * Unmarshal a single Message instance into the base class MessagePtr passed in.
             * @param[out] - message, the pointer into which the Message is unmarshalled.
             * @param[in] - buffer, holds the data to unmarshal
             * @param[in] - bufferSize, the size of the buffer passed in.
             * @return - true on success, false otherwise.
             */
            static bool unmarshalPayload(
                    event::MessagePtr &message, 
                    const char *buffer, 
                    const size_t &bufferSize);

            /**
             * Unmarshal a vector of Messages into the vector reference passed in.
             * @param[out] - message, the pointer into which the Message is unmarshalled.
             * @param[in,out] - numMessages, in: the number of messages in the buffer, out: the number of 
             *  messages unmarshaled. 
             * @param[in] - buffer, holds the data to unmarshal
             * @param[in] - bufferSize, the size of the buffer passed in.
             * @return - true on successfully unmarshaling all messages, false otherwise (numMessages will contain
             *  the number of messages sucessfully unmarshalled on false). 
             */
            static bool unmarshalPayload(
                    std::vector<event::MessagePtr> &messages, 
                    unsigned short &numOfMessages, 
                    const char *buffer, 
                    const size_t &bufferSize);

            /**
             * Marshal a single Message into a NetworkMarshalBuffer. 
             * @param[in] - Message, the Message to marshal.
             * @param[out] - outbuffers, the seraialized instance of message. 
             * @return - true on success, false otherwise.
             */
            static bool marshalPayload(
                    const event::MessagePtr &message, 
                    NetworkMarshalBuffers &outBuffers);

            /**
             * Marshal some number of Messages into a NetworkMarshalBuffer. 
             * @param[in] - Message, the Message to marshal.
             * @param[out] - outbuffers, the seraialized instance of message. 
             * @return - true on success, false otherwise.
             */
            static bool marshalPayload(
                    const std::vector<event::MessagePtr> &message, 
                    NetworkMarshalBuffers &outBuffers);

            // Public as someone has to know how many bytes to read from a 
            // socket somewhere.
            enum 
            { 
                header_length = 
                    sizeof(unsigned short) +   /// Payload length 
                    sizeof(unsigned short)     /// Number of messages
            };

        private:

            DECLARE_LOGGER();

            DataMarshaller(); 
            DataMarshaller(const DataMarshaller &nocopiesthanks); 
            ~DataMarshaller(); 

    }; // class DataMarshaller
} // namespace watcher

#endif // THERES_A_NEW_DATA_MARHSAL_IN_TOWN_H
