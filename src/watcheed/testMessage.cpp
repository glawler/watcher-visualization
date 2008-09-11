#include "testMessage.h"

using namespace std;
using namespace watcher;

INIT_LOGGER(TestMessage, "Message.TestMessage");

TestMessage::TestMessage() : 
    Message(TEST_MESSAGE_TYPE, TEST_MESSAGE_VERSION)
{
    TRACE_ENTER();
    TRACE_EXIT();
}

TestMessage::TestMessage(const string &str, const vector<int> ints) :
    Message(TEST_MESSAGE_TYPE, TEST_MESSAGE_VERSION),
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

ostream &TestMessage::operator<<(ostream &out) const
{
    TRACE_ENTER();

    Message::operator<<(out); 
    out << " stringData: " << stringData;
    out << " intsData:[";
    // Compiler cant' find this: copy(intsData.begin(), intsData.end(), ostream_iterator<int>(out,","));
    for (vector<int>::const_iterator i=intsData.begin(); i != intsData.end(); ++i)
        out << *i << ",";
    out << "] ";

    TRACE_EXIT();
    return out;
}

ostream &watcher::operator<<(ostream &out, const TestMessage &mess)
{
    TRACE_ENTER();
    mess.operator<<(out);
    TRACE_EXIT();
    return out;
}


