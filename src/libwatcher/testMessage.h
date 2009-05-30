#ifndef TEST_MESSAGE_DATA_H
#define TEST_MESSAGE_DATA_H

#include <string>

#include "message.h"

namespace watcher 
{
    namespace event {
        class TestMessage : public Message
        {
            public:
                // The data
                std::string stringData;
                std::vector<int> intsData;

                TestMessage();
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

        std::ostream &operator<<(std::ostream &out, const TestMessage &mess);
    }
}

#endif // TEST_MESSAGE_DATA_H
