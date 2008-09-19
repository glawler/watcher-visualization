#ifndef THERES_A_NEW_DATA_MARHSAL_IN_TOWN_H
#define THERES_A_NEW_DATA_MARHSAL_IN_TOWN_H

#include <iomanip>
#include <string>
#include <sstream>
#include <vector>

#include <boost/noncopyable.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/asio.hpp>

#include "message.h"

#include <boost/serialization/export.hpp>
#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>
#include <boost/archive/polymorphic_binary_iarchive.hpp>
#include <boost/archive/polymorphic_binary_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp> 
#include <boost/serialization/vector.hpp> 

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

            // unmarhsal will unmarshal any type that supports the boost::serialize
            // interface. 
            //
            template<class T>
            boost::logic::tribool unmarshal(T &t, const char *buffer, const size_t &bufferSize, size_t &bytesUsed)
                {
                    TRACE_ENTER(); 
                    // logger is crashing if given anything other than a string!
                    LOG_DEBUG("Unmarshalling a buffer of " << boost::lexical_cast<std::string>(bufferSize) << " bytes.");
                    if (bufferSize < header_length)
                    {
                        // GTL LOG ERROR
                        // set boost::error
                        TRACE_EXIT_RET("false");
                        return false;
                    }
                    std::string inbound_header(buffer, buffer + header_length);
                    std::stringstream is(inbound_header);
                    std::size_t inbound_data_size = 0;
                    if (!(is >> std::hex >> inbound_data_size))
                    {
                        // Header doesn't seem to be valid. Inform the caller.
                        bytesUsed=bufferSize; // GTL Just give up on this read...
                        TRACE_EXIT_RET("false");
                        return false;
                    }
                    LOG_DEBUG("Read header - now reading data of size " << boost::lexical_cast<std::string>(inbound_data_size));
                    if (inbound_data_size > bufferSize)
                    {
                        LOG_ERROR("Packet did not contain entire message."); 
                        // set boost::error
                        // Not enough data read = inderimnate
                        TRACE_EXIT_RET("indeterminate_value");
                        bytesUsed=0;    // GTL hopefully we'll be called again with the whole thing.
                        return boost::logic::tribool::indeterminate_value;
                    }
                    inbound_data_.resize(inbound_data_size);
                    // Extract the data structure from the data just received.
                    try
                    {
                        std::string archive_data(buffer + header_length, inbound_data_size);
                        LOG_DEBUG("Unmarshalled payload - size: " << boost::lexical_cast<std::string>(archive_data.size()) << " data: " << archive_data);
                        std::stringstream archive_stream(archive_data);
                        boost::archive::polymorphic_text_iarchive archive(archive_stream);
                        archive >> t;
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
                        bytesUsed=bufferSize; // GTL Just give up on this read...
                        TRACE_EXIT_RET("false");
                        return false;
                    }

                    bytesUsed=header_length+inbound_data_size;
                    TRACE_EXIT_RET("true");
                    return true;
                }

                template<class T>
                bool marshal(const T &t, std::vector<boost::asio::const_buffer> &outBuffers)
                { 
                    TRACE_ENTER();

                    // Serialize the data first so we know how large it is.
                    std::stringstream archive_stream;
                    boost::archive::polymorphic_text_oarchive archive(archive_stream);
                    archive << t;
                    outbound_data_ = archive_stream.str(); 

                    // Format the header.
                    std::ostringstream header_stream;
                    header_stream << std::setw(header_length) << std::hex << outbound_data_.size();
                    if (!header_stream || header_stream.str().size() != header_length)
                    {
                        TRACE_EXIT_RET("false");
                        return false;
                    }
                    outbound_header_ = header_stream.str();

                    // We can print it as a string as long as we're using text_archive.
                    {
                        LOG_DEBUG("Outbound payload - size: " << boost::lexical_cast<std::string>(outbound_data_.size()) << " data: " << outbound_data_);
                    }

                    outBuffers.push_back(boost::asio::buffer(outbound_header_));
                    outBuffers.push_back(boost::asio::buffer(outbound_data_));

                    TRACE_EXIT_RET("true");
                    return true;
                }

        private:

            DECLARE_LOGGER();

            /// The size of a fixed length header.
            enum { header_length = 8 };

            /// Holds an outbound header.
            std::string outbound_header_;

            /// Holds the outbound data.
            std::string outbound_data_;

            // Holds an inbound header.
            char inbound_header_[header_length];

            /// Holds the inbound data.
            std::vector<char> inbound_data_;

    }; // class DataMarshaller
} // namespace watcher

#endif // THERES_A_NEW_DATA_MARHSAL_IN_TOWN_H

