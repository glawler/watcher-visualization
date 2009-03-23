#ifndef WATCHER_GLOBAL_FUNCTIONS
#define WATCHER_GLOBAL_FUNCTIONS

namespace boost {
    namespace archive {
        class polymorphic_iarchive;
        class polymorphic_oarchive;
    }
    namespace asio
    {
        namespace ip
        {
            class address;
        }
    }
}

//
// These are various functions that are needed, but don't really belong anywhere else, like
// serializations for 3rd party objects. 
//

// Serialize asio addresses.
namespace boost
{
    namespace serialization
    {
        void serialize(boost::archive::polymorphic_iarchive & ar, boost::asio::ip::address &address, const unsigned int file_version);
        void serialize(boost::archive::polymorphic_oarchive & ar, boost::asio::ip::address &address, const unsigned int file_version);
    }
}


#endif //  WATCHER_GLOBAL_FUNCTIONS
