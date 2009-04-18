#ifndef message_fwd_h
#define message_fwd_h

namespace watcher {
    namespace event {
        class Message;
        typedef boost::shared_ptr<Message> MessagePtr; 
    }
}

#endif
