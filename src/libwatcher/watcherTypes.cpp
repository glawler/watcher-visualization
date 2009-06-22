#include <sys/time.h>
#include <stdlib.h>
#include "watcherTypes.h"

namespace watcher 
{
    Timestamp getCurrentTime()
    {
        // Is there a BOOST call for this?
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return (Timestamp)tv.tv_sec * 1000 + (Timestamp)tv.tv_usec/1000;
    }

}
