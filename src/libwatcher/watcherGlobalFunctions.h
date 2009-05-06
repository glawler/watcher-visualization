#ifndef WATCHER_GLOBAL_FUNCTIONS
#define WATCHER_GLOBAL_FUNCTIONS

#include <boost/serialization/split_free.hpp>
#include "watcherTypes.h"

BOOST_SERIALIZATION_SPLIT_FREE(boost::asio::ip::address);

namespace boost 
{
    namespace serialization 
    {
        template<class Archive>
            void save(Archive & ar, const watcher::NodeIdentifier &a, const unsigned int /* version */)
            {
                std::string tmp=a.to_string();
                ar & tmp;
            }

        template<class Archive>
            void load(Archive & ar, watcher::NodeIdentifier &a, const unsigned int /* version */)
            {
                std::string tmp;
                ar & tmp;
                a=boost::asio::ip::address::from_string(tmp);
            }
    }
}

#endif //  WATCHER_GLOBAL_FUNCTIONS
