#include <boost/foreach.hpp>

#include "watcherSerialize.h"
#include "dataPointMessage.h"

using namespace std;
using namespace boost;

namespace watcher {
    namespace event {
        INIT_LOGGER(DataPointMessage, "Message.DataPointMessage");

        DataPointMessage::DataPointMessage() : 
            Message(DATA_POINT_MESSAGE_TYPE, DATA_POINT_MESSAGE_VERSION),
            dataName(),
            dataPoints()
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }


        DataPointMessage::DataPointMessage(const std::string &dn, const DataPointList &data) : 
            Message(COLOR_MESSAGE_TYPE, COLOR_MESSAGE_VERSION),
            dataName(dn), 
            dataPoints(data)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        DataPointMessage::DataPointMessage(const DataPointMessage &other) : 
            Message(other),
            dataName(other.dataName), 
            dataPoints(other.dataPoints)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        bool DataPointMessage::operator==(const DataPointMessage &other) const
        {
            TRACE_ENTER();

            bool retVal = 
                Message::operator==(other) && 
                dataName==other.dataName && 
                dataPoints==other.dataPoints;

            TRACE_EXIT_RET(retVal);
            return retVal;
        }

        DataPointMessage &DataPointMessage::operator=(const DataPointMessage &other)
        {
            TRACE_ENTER();

            Message::operator=(other);
            dataName=other.dataName;
            dataPoints=other.dataPoints;

            TRACE_EXIT();
            return *this;
        }

        // virtual 
        std::ostream &DataPointMessage::toStream(std::ostream &out) const
        {
            TRACE_ENTER();

            Message::toStream(out);
            out << " name: " << dataName; 
            out << " data: ["; 
            BOOST_FOREACH(double point, dataPoints)
                out << " " << point;
            out << "]"; 

            TRACE_EXIT();
            return out;
        }

        ostream& operator<<(ostream &out, const DataPointMessage &mess)
        {
            TRACE_ENTER();
            mess.operator<<(out);
            TRACE_EXIT();
            return out;
        }

        template <typename Archive> void DataPointMessage::serialize(Archive& ar, const unsigned int /* file_version */)
        {
            TRACE_ENTER();
            ar & boost::serialization::base_object<Message>(*this);
            ar & dataName;
            ar & dataPoints;
            TRACE_EXIT();
        }
    }
}

BOOST_CLASS_EXPORT(watcher::event::DataPointMessage);
