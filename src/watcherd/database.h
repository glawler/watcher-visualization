/**@file
 * @author Michael.Elkins@cobham.com
 * @date 2009-05-04
 */

#ifndef database_h
#define database_h

#include <boost/utility.hpp>

namespace watcher
{
    /** Abstraction of a database handle. */
    class DatabaseHandle : private boost::noncopyable
    {
        public:
            virtual ~DatabaseHandle() = 0;

            static DatabaseHandle* connect(const std::string&);
    };
} //namespace

#endif /* database_h */
