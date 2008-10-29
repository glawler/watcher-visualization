#include "singletonConfig.h"

using namespace watcher;
using namespace libconfig;

pthread_mutex_t singletonConfig::accessMutex=PTHREAD_MUTEX_INITIALIZER;

INIT_LOGGER(singletonConfig, "singletonConfig");

// static
singletonConfig &singletonConfig::instance()
{
    TRACE_ENTER();
    static singletonConfig theOneAndOnlyConfigObjectYouBetcha;
    TRACE_EXIT();
    return theOneAndOnlyConfigObjectYouBetcha;
}

singletonConfig::singletonConfig()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

singletonConfig::~singletonConfig()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

libconfig::Config &singletonConfig::getConfig()
{
    TRACE_ENTER();
    TRACE_EXIT();
    return cfg;
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

void singletonConfig::setConfigFile(const std::string &filename_)
{
    TRACE_ENTER();
    filename=filename_;
    TRACE_EXIT();
}
void singletonConfig::saveConfig()
{
    TRACE_ENTER();
    cfg.writeFile(filename.data());
    TRACE_EXIT();
}
