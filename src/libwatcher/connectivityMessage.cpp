/**
 * @file connectivityMessage.h
 * @author Geoff Lawler <Geoff.Lawler@cobham.com>
 * @date 2009-05-06
 */
#include <boost/asio.hpp>

#include "watcherSerialize.h"
#include "watcherGlobalFunctions.h"             // for address serialization

#include "connectivityMessage.h"

using namespace std;

namespace watcher 
{
    namespace event 
    {
        INIT_LOGGER(ConnectivityMessage, "Message.ConnectivityMessage");

        ConnectivityMessage::ConnectivityMessage() : 
            Message(CONNECTIVITY_MESSAGE_TYPE, CONNECTIVITY_MESSAGE_VERSION), 
            neighbors(),
            layer(PHYSICAL_LAYER)

        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        // virtual
        ConnectivityMessage::~ConnectivityMessage()  
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }
        ConnectivityMessage::ConnectivityMessage(const ConnectivityMessage &other) :
            Message(other.type, other.version), 
            neighbors(other.neighbors),
            layer(PHYSICAL_LAYER)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        bool ConnectivityMessage::operator==(const ConnectivityMessage &other) const
        {
            TRACE_ENTER();

            bool retVal = 
                Message::operator==(other) && 
                neighbors == other.neighbors &&
                layer == other.layer;

            TRACE_EXIT_RET(retVal);
            return retVal;
        }

        ConnectivityMessage &ConnectivityMessage::operator=(const ConnectivityMessage &other)
        {
            TRACE_ENTER();

            Message::operator=(other);
            neighbors=other.neighbors;
            layer=other.layer;

            TRACE_EXIT();
            return *this;
        }

        // virtual 
        std::ostream &ConnectivityMessage::toStream(std::ostream &out) const
        {
            TRACE_ENTER();

            Message::toStream(out);
            out << " layer: " << layer;
            out << " neighbors:[";
            for (vector<NodeIdentifier>::const_iterator i=neighbors.begin(); i != neighbors.end(); ++i)
                out << *i << ",";
            out << "] ";

            TRACE_EXIT();
            return out;
        }

        ostream &operator<<(ostream &out, const ConnectivityMessage &mess)
        {
            TRACE_ENTER();
            mess.operator<<(out);
            TRACE_EXIT();
            return out;
        }

        template <typename Archive> void ConnectivityMessage::serialize(Archive & ar, const unsigned int /* file_version */)
        {
            TRACE_ENTER();
            ar & boost::serialization::base_object<Message>(*this);
            ar & neighbors;
            ar & layer;
            TRACE_EXIT();
        }
    }
}

BOOST_CLASS_EXPORT(watcher::event::ConnectivityMessage);

