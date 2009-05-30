#include "displayInfo.h"

using namespace watcher;

INIT_LOGGER(DisplayInfo, "DisplayInfo"); 

DisplayInfo::DisplayInfo(const std::string &category) :
    categoryName(category) 
{
    TRACE_ENTER();
    TRACE_EXIT();
}

// virtual
DisplayInfo::~DisplayInfo()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

