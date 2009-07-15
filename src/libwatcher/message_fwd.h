/**
 * @file message_fwd.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
#ifndef message_fwd_h
#define message_fwd_h

namespace watcher {
    namespace event {
        class Message;
        typedef boost::shared_ptr<Message> MessagePtr; 
    }
}

#endif
