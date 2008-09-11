#ifndef TEST_MESSAGE_DATA_H
#define TEST_MESSAGE_DATA_H

#include <string>
#include <iostream>
#include <boost/serialization/vector.hpp>

#include "message.h"
#include "logger.h"

namespace watcher 
{
    class TestMessage : public Message
    {
        public:
            // The data
            std::string stringData;
            std::vector<int> intsData;

            TestMessage();
            TestMessage(const std::string &str, const std::vector<int> ints);
            TestMessage(const TestMessage &other);

            bool operator==(const TestMessage &other) const;
            TestMessage &operator=(const TestMessage &other);

            std::ostream &operator<<(std::ostream &out) const;

            template <class Archive>
                void serialize(Archive &ar, const unsigned int /*version*/)
                {
                    TRACE_ENTER();
                    ar.template register_type<Message>();
                    ar & boost::serialization::base_object<Message>(*this);
                    ar & stringData;
                    ar & intsData;
                    TRACE_EXIT();
                }

            DECLARE_LOGGER();
    };


    std::ostream &operator<<(std::ostream &out, const TestMessage &mess);
}

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_GUID(watcher::TestMessage, "TestMessage")

#endif // TEST_MESSAGE_DATA_H
