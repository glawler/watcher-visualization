/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Affero General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Affero General Public License for more details.
 * 
 *     You should have received a copy of the GNU Affero General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file testMessage.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
#ifndef TEST_MESSAGE_DATA_H
#define TEST_MESSAGE_DATA_H

#include <string>

#include "message.h"

namespace watcher {
    namespace event {
        /// a no-op event for system testing
        class TestMessage : public Message
        {
            public:
                // The data
                std::string stringData;
                std::vector<int> intsData;

                TestMessage();
                /** Construct a test message consisting of a string and a vector of integers.
                 * Useful for generating distinct messages.
                 * @param[in] str string to include in the event
                 * @param[in] ints vector of integers to include in the event
                 */
                TestMessage(const std::string &str, const std::vector<int>& ints);
                TestMessage(const TestMessage &other);

                bool operator==(const TestMessage &other) const;
                TestMessage &operator=(const TestMessage &other);

                virtual std::ostream &toStream(std::ostream &out) const;
                std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

            private:
                friend class boost::serialization::access;
                template <typename Archive> void serialize(Archive & ar, const unsigned int file_version);
                DECLARE_LOGGER();
        };

        typedef boost::shared_ptr<TestMessage> TestMessagePtr; 

        /** Write human readable version of a TestMessage to the output stream.
         * @param out the output stream
         * @param[in] mess the test message
         * @return reference to the output stream.
         */
        std::ostream &operator<<(std::ostream &out, const TestMessage &mess);
    }
}

#endif // TEST_MESSAGE_DATA_H
