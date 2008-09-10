#ifndef TEST_MESSAGE_DATA_H
#define TEST_MESSAGE_DATA_H

#include <string>
#include <vector>
#include "baseMessage.h"
#include "logger.h"

namespace watcher 
{
    class TestMessage : public BaseMessage
    {
        public:
            // The data
            std::string stringData;
            std::vector<int> intsData;

            TestMessage(const std::string &str, const std::vector<int> ints) 
                : BaseMessage(TEST_MESSAGE_VERSION, TEST_MESSAGE_TYPE),
                stringData(str),
                intsData(ints)
            {
                TRACE_ENTER();
                LOG_DEBUG("debug statment");
                LOG_INFO("info statement");
                TRACE_EXIT();
            }

            DECLARE_LOGGER();
    };

    INIT_LOGGER(TestMessage, "BaseMessage.TestMessage");

}

#endif // TEST_MESSAGE_DATA_H
