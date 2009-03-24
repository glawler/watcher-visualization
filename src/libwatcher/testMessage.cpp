#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/export.hpp>

#include "testMessage.h"

using namespace std;
namespace watcher {
    namespace event {

        INIT_LOGGER(TestMessage, "Message.TestMessage");
        BOOST_CLASS_EXPORT_GUID(TestMessage, "TestMessage");

        TestMessage::TestMessage() : Message(TEST_MESSAGE_TYPE, MESSAGE_TEST_VERSION)
        {
            TRACE_ENTER();
            stringData="Hello world";
            intsData.push_back(1);
            intsData.push_back(666);
            intsData.push_back(2);
            intsData.push_back(666);
            intsData.push_back(3);
            intsData.push_back(668);
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

        TestMessage::TestMessage(const TestMessage &other)
        {
            TRACE_ENTER();
            (*this)=other;
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
