/**
 * @file connection_fwd.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
#ifndef connection_fwd_h
#define connection_fwd_h

namespace watcher
{
    class Connection;

    typedef boost::shared_ptr<Connection> ConnectionPtr;

} //namespace

#endif /* connection_fwd_h */
