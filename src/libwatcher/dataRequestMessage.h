#ifndef DATA_REQUEST_MESSAGE_H
#define DATA_REQUEST_MESSAGE_H

#include <list>
#include "message.h"

namespace watcher {
    namespace event {
        class DataRequestMessage : public Message {
            public:

                // The messages that you are interested in getting
                // Leave empty for all messages. 
                //
                typedef std::list<MessageType> MessageTypeList;
                MessageTypeList dataTypesRequested;

                // Data Requests starting at this time period in Epoch milliseconds.
                Timestamp startingAt;

                // How quickly do you want the Messages fed to you?
                // 1=real time, 2=twice as fast, .5=half as fast, etc.
                // Negative numbers mean get data in reverse time. 'Course
                // reverse time is only good back to start of test as forward time
                // is only good to current time or end of test. Zero is meaningless
                // unless you're The Doctor.
                float timeFactor; 

                // Layer filter: only get those messages that are on these layers.
                // Ex: layers = HIERARCHY_LAYER & NODE_LAYER;
                // Keep it empty (0) to request all layers.
                unsigned int layers; 

                DataRequestMessage(); 

                DataRequestMessage(
                                   const MessageTypeList &types,
                                   const Timestamp &startAt_=0, 
                                   const float &factor_=1,
                                   const unsigned int layers_=0); 

                DataRequestMessage(const DataRequestMessage &other);

                virtual ~DataRequestMessage();

                void requestAllMessages();  // Get all known messages. 

                bool operator==(const DataRequestMessage &other) const;
                DataRequestMessage &operator=(const DataRequestMessage &other);

                virtual std::ostream &toStream(std::ostream &out) const;
                std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

            protected:
            private:
                friend class boost::serialization::access;
                template <typename Archive>
                void serialize(Archive & ar, const unsigned int file_version)
                {
                    TRACE_ENTER();
                    ar & boost::serialization::base_object<Message>(*this);
                    ar & dataTypesRequested;
                    ar & startingAt;
                    ar & timeFactor;
                    ar & layers;
                    TRACE_EXIT();
                }

                DECLARE_LOGGER();
        };

        typedef boost::shared_ptr<DataRequestMessage> DataRequestMessagePtr;
        std::ostream &operator<<(std::ostream &out, const DataRequestMessage &mess);
    }
}
#endif // DATA_REQUEST_MESSAGE_H
