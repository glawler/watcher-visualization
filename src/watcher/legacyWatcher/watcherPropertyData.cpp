#include "watcherPropertyData.h"

WatcherPropertyData *findWatcherPropertyData(unsigned int index, WatcherPropertiesList &theList)
{
    WatcherPropertiesList::iterator i;
    for (i=theList.begin(); i!=theList.end(); i++)
        if ( (*i)->identifier==index)
            return *i;
    return NULL;
}

