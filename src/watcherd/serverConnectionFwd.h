#ifndef server_connection_fwd_h
#define server_connection_fwd_h

namespace watcher
{
    class ServerConnection;

    typedef boost::shared_ptr<ServerConnection> ServerConnectionPtr;
    typedef boost::weak_ptr<ServerConnection> ServerConnectionWeakPtr;
} //namespace

#endif /* server_connection_fwd_h */
