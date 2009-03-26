#ifndef WATCHER_GLOBAL_FUNCTIONS
#define WATCHER_GLOBAL_FUNCTIONS

#include <boost/asio/ip/address.hpp>
#include <boost/serialization/split_free.hpp>

BOOST_SERIALIZATION_SPLIT_FREE(boost::asio::ip::address);

namespace boost 
{
    namespace serialization 
    {
        template<class Archive>
            void save(Archive & ar, const boost::asio::ip::address & a, const unsigned int version)
            {
                std::string tmp=a.to_string();
                ar & tmp;
            }

        template<class Archive>
            void load(Archive & ar, boost::asio::ip::address & a, const unsigned int version)
            {
                std::string tmp;
                ar & tmp;
                a=boost::asio::ip::address::from_string(tmp);
            }
    }
}

#endif //  WATCHER_GLOBAL_FUNCTIONS
