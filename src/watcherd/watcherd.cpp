#include "watcherd.h"

using namespace watcher;
using namespace std;
using namespace boost;

INIT_LOGGER(Watcherd, "Watcherd"); 

WatcherdPtr getWatcherd()
{
    TRACE_ENTER();
    static WatcherdPtr theWatcherd(new Watcherd);
    TRACE_EXIT();
    return theWatcherd;
}


