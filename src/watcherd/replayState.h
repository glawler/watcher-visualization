#ifndef replay_state_h
#define replay_state_h

#include <boost/scoped_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

// forward decls
namespace boost {
    namespace system {
        class error_code;
    }
}

namespace watcher {

    /* forward decls */
    class ServerConnection;
    typedef boost::shared_ptr<ServerConnection> ServerConnectionPtr;

    class ReplayState : public boost::enable_shared_from_this<ReplayState> {
        public:
            struct Bad_speed {};
            struct Bad_buffer_size {};
            struct Bad_step {};

            ReplayState(ServerConnectionPtr& ptr, float speed = 1.0f);
            void run();
            void timer_handler(const boost::system::error_code& error);
            
            /** Adjust the event playback speed.
             * @param[in] f a floating point value representing the speed multiplication factor
             */
            ReplayState& speed(float f);

            /** Adjust the number of events prefetched from the database. */
            ReplayState& buffer_size(unsigned int n);

            /** Adjust the granularity of the timer used to bundle events.
             * @param[in] n positive integer representing the number of milliseconds
             */
            ReplayState& time_step(unsigned int n);

        private:
            struct impl;
            boost::scoped_ptr<impl> impl_;
    };

}

#endif /* replay_state_h */
