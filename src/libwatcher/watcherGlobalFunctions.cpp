#include <boost/asio.hpp>

#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>
#include <boost/serialization/string.hpp>           // for address string respresentation
#include <boost/serialization/export.hpp>

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

