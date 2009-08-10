/**
 * @file dataMarshaller.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
#ifndef THERES_A_NEW_DATA_MARHSAL_IN_TOWN_H
#define THERES_A_NEW_DATA_MARHSAL_IN_TOWN_H

#include <string>
#include <deque>

#include <boost/asio.hpp>

#include "libwatcher/message.h"

namespace watcher 
{
    /// A reference-counted non-modifiable buffer class.
    class shared_const_buffer
    {
        public:
            /// Construct from a std::string.
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

            
            ///  a sequence of buffers used to write to the network.
            typedef shared_const_buffer NetworkMarshalBuffer;
            // typedef std::vector<NetworkMarshalBuffer> NetworkMarshalBuffers;
            typedef std::deque<NetworkMarshalBuffer> NetworkMarshalBuffers;
            typedef boost::shared_ptr<NetworkMarshalBuffers> NetworkMarshalBuffersPtr;
/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 * 
 *     You should have received a copy of the GNU General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */


            /**
             * Unmarshal a header, returning the length of the payload which follows. 
             * @param[in] buffer the buffer which contains the header data
             * @param[in] bufferSize the size of 'buffer'
             * @param[out] payloadSize the size of the payload that the header is attached to.
             * @param[out] messageNum the number of messages in the payload.
             * @return true on success, false otherwise.
             */
            static bool unmarshalHeader(
                    const char *buffer, 
                    const size_t &bufferSize, 
                    size_t &payloadSize, 
                    unsigned short &messageNum);

            /**
             * Unmarshal a single Message instance into the base class MessagePtr passed in.
             * @param[out] message the pointer into which the Message is unmarshalled.
             * @param[in] buffer holds the data to unmarshal
             * @param[in] bufferSize the size of the buffer passed in.
             * @retval true on success
             * @retval false otherwise
             */
            static bool unmarshalPayload(
                    event::MessagePtr &message, 
                    const char *buffer, 
                    const size_t &bufferSize);

            /**
             * Unmarshal a vector of Messages into the vector reference passed in.
             * @param[out] messages the pointer into which the Messages are unmarshalled.
             * @param[in,out] numOfMessages in: the number of messages in the buffer, out: the number of 
             *  messages unmarshaled. 
             * @param[in] buffer holds the data to unmarshal
             * @param[in] bufferSize the size of the buffer passed in.
             * @retval true on successfully unmarshaling all messages,
             * @retval false otherwise (numMessages will contain
             *  the number of messages sucessfully unmarshalled on false). 
             */
            static bool unmarshalPayload(
                    std::vector<event::MessagePtr> &messages, 
                    unsigned short &numOfMessages, 
                    const char *buffer, 
                    const size_t &bufferSize);

            /**
             * Marshal a single Message into a NetworkMarshalBuffer. 
             * @param[in] message the Message to marshal.
             * @param[out] outBuffers the seraialized instance of message. 
             * @return true on success, false otherwise.
             */
            static bool marshalPayload(
                    const event::MessagePtr &message, 
                    NetworkMarshalBuffers &outBuffers);

            /**
             * Marshal some number of Messages into a NetworkMarshalBuffer. 
             * @param[in] message the Message to marshal.
             * @param[out] outBuffers the seraialized instance of message. 
             * @return true on success, false otherwise.
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

