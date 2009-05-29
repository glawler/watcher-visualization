#include "singletonConfig.h"

using namespace watcher;
using namespace libconfig;

pthread_mutex_t SingletonConfig::accessMutex=PTHREAD_MUTEX_INITIALIZER;

INIT_LOGGER(SingletonConfig, "SingletonConfig");

// static
Config &SingletonConfig::instance()
{
    TRACE_ENTER();
    static Config theInstance;
    TRACE_EXIT();
    return theInstance;
}

void SingletonConfig::lock()
{
    TRACE_ENTER();
    pthread_mutex_lock(&accessMutex);
    TRACE_EXIT();
}

void SingletonConfig::unlock()
{
    TRACE_ENTER();
    pthread_mutex_unlock(&accessMutex);
    TRACE_EXIT();
}

