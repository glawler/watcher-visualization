#include <boost/asio.hpp>

#include "watcherSerialize.h"
#include "watcherGlobalFunctions.h"

// void boost::serialization::serialize(boost::archive::polymorphic_iarchive & ar, boost::asio::ip::address &address, const unsigned int /* file_version */)
// {
//     std::string tmp;
//     ar & tmp;
//     address=boost::asio::ip::address::from_string(tmp);
// }
// 
// void boost::serialization::serialize(boost::archive::polymorphic_oarchive & ar, boost::asio::ip::address &address, const unsigned int /* file_version */)
// {
//     std::string tmp=address.to_string();
//     ar & tmp;
// }

