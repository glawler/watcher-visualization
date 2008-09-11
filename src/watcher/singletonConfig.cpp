#include "singletonConfig.h"

using namespace watcher;
using namespace libconfig;

pthread_mutex_t singletonConfig::accessMutex=PTHREAD_MUTEX_INITIALIZER;

INIT_LOGGER(singletonConfig, "singletonConfig");

// static
Config &singletonConfig::instance()
{
    TRACE_ENTER();
    static Config theInstance;
    TRACE_EXIT();
    return theInstance;
}

void singletonConfig::lock()
{
    TRACE_ENTER();
    pthread_mutex_lock(&accessMutex);
    TRACE_EXIT();
}

void singletonConfig::unlock()
{
    TRACE_ENTER();
    pthread_mutex_unlock(&accessMutex);
    TRACE_EXIT();
}

