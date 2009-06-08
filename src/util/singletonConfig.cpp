#include "singletonConfig.h"

using namespace watcher;
using namespace libconfig;

pthread_mutex_t SingletonConfig::accessMutex=PTHREAD_MUTEX_INITIALIZER;
std::string SingletonConfig::filename=""; 

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

void SingletonConfig::setConfigFile(const std::string &filename_)
{
    TRACE_ENTER();
    filename=filename_;
    TRACE_EXIT();
}

void SingletonConfig::saveConfig()
{
    TRACE_ENTER();
    instance().writeFile(filename.data());
    TRACE_EXIT();
}
