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
 * @file testMessage.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15
 */
#include "watcherSerialize.h"
#include "testMessage.h"

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

        TestMessage::TestMessage(const std::string &str, const std::vector<int>& ints) :
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

#if 0
        TestMessage &TestMessage::operator=(const TestMessage &other)
        {
            TRACE_ENTER();

            Message::operator=(other);
            stringData=other.stringData;
            intsData=other.intsData;

            TRACE_EXIT();
            return *this;
        }
#endif

        // virtual 
        std::ostream &TestMessage::toStream(std::ostream &out) const
        {
            TRACE_ENTER();

            Message::toStream(out);
            out << " stringData:\"" << stringData << "\""; 
            out << " intsData:[";
            // Compiler cant' find this: copy(intsData.begin(), intsData.end(), ostream_iterator<int>(out,","));
            for (std::vector<int>::const_iterator i=intsData.begin(); i != intsData.end(); ++i)
                out << *i << ",";
            out << "] ";

            TRACE_EXIT();
            return out;
        }

        std::ostream &operator<<(std::ostream &out, const TestMessage &mess)
        {
            TRACE_ENTER();
            mess.operator<<(out);
            TRACE_EXIT();
            return out;
        }

        template <typename Archive> void TestMessage::serialize(Archive & ar, const unsigned int /* file_version */)
        {
            TRACE_ENTER();
            ar & boost::serialization::base_object<Message>(*this);
            ar & stringData;
            ar & intsData;
            TRACE_EXIT();
        }
    }
}

BOOST_CLASS_EXPORT(watcher::event::TestMessage);
