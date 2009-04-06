#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/export.hpp>

#include "testMessage.h"

BOOST_CLASS_EXPORT_GUID(watcher::event::TestMessage, "watcher::event::TestMessage");

using namespace std;
namespace watcher {
    namespace event {

        INIT_LOGGER(TestMessage, "Message.TestMessage");

        TestMessage::TestMessage() : 
            Message(TEST_MESSAGE_TYPE, MESSAGE_TEST_VERSION), 
            stringData(),
            intsData()
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        TestMessage::TestMessage(const string &str, const vector<int> ints) :
            Message(TEST_MESSAGE_TYPE, MESSAGE_TEST_VERSION),
            stringData(str),
            intsData(ints)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        TestMessage::TestMessage(const TestMessage &other) :
            Message(other.type, other.version), 
            stringData(other.stringData),
            intsData(other.intsData)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        bool TestMessage::operator==(const TestMessage &other) const
        {
            TRACE_ENTER();

            bool retVal = 
                Message::operator==(other) && 
                stringData == other.stringData &&
                intsData == other.intsData;

            TRACE_EXIT_RET(retVal);
            return retVal;
        }

        TestMessage &TestMessage::operator=(const TestMessage &other)
        {
            TRACE_ENTER();

            Message::operator=(other);
            stringData=other.stringData;
            intsData=other.intsData;

            TRACE_EXIT();
            return *this;
        }

        // virtual 
        std::ostream &TestMessage::toStream(std::ostream &out) const
        {
            TRACE_ENTER();

            Message::toStream(out);
            out << " stringData:\"" << stringData << "\""; 
            out << " intsData:[";
            // Compiler cant' find this: copy(intsData.begin(), intsData.end(), ostream_iterator<int>(out,","));
            for (vector<int>::const_iterator i=intsData.begin(); i != intsData.end(); ++i)
                out << *i << ",";
            out << "] ";

            TRACE_EXIT();
            return out;
        }

        ostream &operator<<(ostream &out, const TestMessage &mess)
        {
            TRACE_ENTER();
            mess.operator<<(out);
            TRACE_EXIT();
            return out;
        }
    }
}
