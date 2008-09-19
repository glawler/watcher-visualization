#ifndef TEST_MESSAGE_DATA_H
#define TEST_MESSAGE_DATA_H

#include <string>

#include "logger.h"
#include "message.h"

namespace boost {
    namespace archive {
        class polymorphic_iarchive;
        class polymorphic_oarchive;
    }
}

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

            virtual std::ostream &toStream(std::ostream &out) const;
            std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

            virtual void serialize(boost::archive::polymorphic_iarchive & ar, const unsigned int file_version);
            virtual void serialize(boost::archive::polymorphic_oarchive & ar, const unsigned int file_version);

        private:
            DECLARE_LOGGER();
    };

    typedef boost::shared_ptr<TestMessage> TestMessagePtr; 

    std::ostream &operator<<(std::ostream &out, const TestMessage &mess);
}

#endif // TEST_MESSAGE_DATA_H
