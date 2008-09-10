#ifndef BASE_MESSAGE_H
#define BASE_MESSAGE_H

#include "messageTypesAndVersions.h"
#include "logger.h"

namespace watcher 
{
    class BaseMessage
    {
        public:
            unsigned int version;
            unsigned int type;

            BaseMessage(const int &v, const int &t) 
                : version(v), type(t)
            {
                TRACE_ENTER();
                TRACE_EXIT();
            }

            // BaseMessage(const BaseMessage &other)
            // {
            //     (*this)=other;
            // }

            ~BaseMessage()
            {
                TRACE_ENTER();
                TRACE_EXIT();
            }

            bool operator==(const BaseMessage &other)
            {
                TRACE_ENTER();
                bool retVal = version==other.version && type==other.type;
                TRACE_EXIT_RET(retVal);
                return retVal;
            }

            BaseMessage &operator=(const BaseMessage &other)
            {
                TRACE_ENTER();
                version=other.version;
                type=other.type;
                TRACE_EXIT();
            }

            std::ostream &operator<<(std::ostream &out)
            {
                return 
                    out << " version: " << version
                        << " type: " << type;
            }

            DECLARE_LOGGER();

        protected:
        private:
    };

    INIT_LOGGER(BaseMessage, "BaseMessage");
}

#endif // BASE_MESSAGE_H
