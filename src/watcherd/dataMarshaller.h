#ifndef THERES_A_NEW_DATA_MARHSAL_IN_TOWN_H
#define THERES_A_NEW_DATA_MARHSAL_IN_TOWN_H

#include <iomanip>
#include <string>
#include <sstream>
#include <vector>

#include <boost/noncopyable.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/asio.hpp>

#include <boost/archive/archive_exception.hpp>
#include <boost/serialization/export.hpp>

#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>
#include <boost/archive/polymorphic_binary_iarchive.hpp>
#include <boost/archive/polymorphic_binary_oarchive.hpp>

#include <boost/serialization/shared_ptr.hpp>   // Need this to serialize shared_ptrs. 
#include <boost/serialization/vector.hpp>        // Need this to serialize std::vectors. 

#include <boost/lexical_cast.hpp>

#include "logger.h"

namespace watcher 
{
    //
    // The DataMarshaller class uses the boost::serialization functionality 
    // to marshal and unmarshal any type T into and outof databuffers. These
    // data buffers are perfect for sending as network packets.  
    //
    class DataMarshaller : private boost::noncopyable
    {
        public:
            DataMarshaller() { }

            /**
             * Unmarshal a header, returning the length of the payload which follows. 
             * @param[in] - buffer, the buffer which contains the header data
             * @param[in] - bufferSize, the size of 'buffer'
             * @param[out] - payloadSize, the size of the payload that the header is attached to.
             * @param[out] - type, the type of the payload
             */
            bool unmarshalHeader(const char *buffer, const size_t &bufferSize, size_t &payloadSize, unsigned int &type)
            {
                TRACE_ENTER();
                payloadSize=0;

                // logger is crashing if given anything other than a string!
                LOG_DEBUG("Unmarshalling a header of " << boost::lexical_cast<std::string>(bufferSize) << " bytes.");
                if (bufferSize < header_length)
                {
                    LOG_ERROR("Not enought data in payload to unmarshal the packet or packet is corrupt"); 
                    // set boost::error
                    TRACE_EXIT_RET("false");
                    return false;
                }
                std::string inbound_header(buffer, buffer + header_length);
                std::stringstream is(inbound_header);
                if (!(is >> std::setw(sizeof(std::string::size_type)) >> std::hex >> payloadSize) || 
                    !(is >> std::setw(sizeof(type)) >> std::hex >> type))
                {
                    // Header doesn't seem to be valid. Inform the caller.
                    LOG_DEBUG("Error reading payloadsize and type from header: "  << strerror(errno)); 
                    TRACE_EXIT_RET("false");
                    return false;
                }
                return true;
            }
            
            /**
             * Unmarshal a payload of type T. Buffer shuld contain the payload portion of a message of size
             * given by unmarshalHeader(). T must be boost::serializable.
             *
             */
            template<class T>
                bool unmarshalPayload(T &t, const char *buffer, const size_t &bufferSize)
                {
                    TRACE_ENTER(); 
                    // logger is crashing if given anything other than a string!
                    LOG_DEBUG("Unmarshalling a payload of " << boost::lexical_cast<std::string>(bufferSize) << " bytes.");

                    // Extract the data structure from the data just received.
                    try
                    {
                        std::string archive_data(buffer, bufferSize);
                        std::stringstream archive_stream(archive_data);
                        boost::archive::polymorphic_text_iarchive archive(archive_stream);
                        archive >> t;
                        LOG_DEBUG("Unmarshalled payload - size: " << boost::lexical_cast<std::string>(archive_data.size()) << 
                                " data: " << archive_data);
                    }
                    catch (boost::archive::archive_exception& e)
                    {
                        // Unable to decode data.
                        // boost::system::error_code error(boost::asio::error::invalid_argument);
                        // boost::get<0>(handler)(error);
                        LOG_WARN("Exception thrown while serializing the message: " << e.what());

                        // GTL - TODO: only diplay message below if the exception is 'unregistered_class'
                        if (e.code == boost::archive::archive_exception::unregistered_class) 
                        {
                            LOG_WARN("Did you link this message's object file into this executable?"); 
                        }
                        TRACE_EXIT_RET("false");
                        return false;
                    }

                    TRACE_EXIT_RET("true");
                    return true;
                }

            /**
             * Marshal an instance of class T.
             * @param[in] - T, the instance to marshal.
             * @param[in] - type, the type of the payload (used to regenerate a T instance when unmarshalling). 
             * @param[out] - outbuffers, the seraialized instance of T.
             */
            template<class T>
                bool marshal(const T &t, unsigned int type, std::vector<boost::asio::const_buffer> &outBuffers)
                { 
                    TRACE_ENTER();

                    // LOG_DEBUG("Serializing message: " << *t); 

                    // Serialize the data first so we know how large it is.
                    try
                    {
                        std::ostringstream archive_stream;
                        boost::archive::polymorphic_text_oarchive pto(archive_stream);
                        pto << t;
                        outbound_data_ = archive_stream.str(); 
                    }
                    catch(boost::archive::archive_exception e)
                    {
                        LOG_WARN("Caught exception when serializing message: " << e.what());  
                        TRACE_EXIT_RET("false");
                        return false;
                    }

                    LOG_DEBUG("Serialized " << outbound_data_.size() << " bytes of message data"); 

                    // Format the header.
                    // GTL - may be nice to put the header itself into a class that supports archive/serialization...
                    std::ostringstream header_stream;
                    header_stream << std::setw(sizeof(std::string::size_type)) << std::hex << outbound_data_.size(); 
                    header_stream << std::setw(sizeof(type))                   << std::hex << type;
                    LOG_DEBUG("header_length: " << header_length << " header_stream:" << header_stream << " size: " << header_stream.str().size() << " sizeof: " << sizeof(outbound_data_.size()) + sizeof(type)); 
                    if (!header_stream || header_stream.str().size() != header_length)
                    {
                        TRACE_EXIT_RET("false");
                        return false;
                    }
                    outbound_header_ = header_stream.str();

                    // We can print it as a string as long as we're using text_archive.
                    {
                        LOG_DEBUG("Outbound payload - size: " << boost::lexical_cast<std::string>(outbound_data_.size()) << 
                                  " data: " << outbound_data_);
                    }

                    outBuffers.push_back(boost::asio::buffer(outbound_header_));
                    outBuffers.push_back(boost::asio::buffer(outbound_data_));

                    TRACE_EXIT_RET("true");
                    return true;
                }

            /** The size of a fixed length header.
             * Can be used to allocate a header buffer.
             */
            enum { header_length = sizeof(std::string::size_type) + sizeof(unsigned int)  };

        private:

            DECLARE_LOGGER();

            /// Holds an outbound header.
            std::string outbound_header_;

            /// Holds the outbound data.
            std::string outbound_data_;

            // Holds an inbound header.
            char inbound_header_[header_length];

    }; // class DataMarshaller
} // namespace watcher

#endif // THERES_A_NEW_DATA_MARHSAL_IN_TOWN_H

