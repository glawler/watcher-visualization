#ifndef WATCHER_LOGGER_H
#define WATCHER_LOGGER_H

#include "log4cxx/logger.h"

#include "log4cxx/basicconfigurator.h"
#include "log4cxx/propertyconfigurator.h"
#include "log4cxx/helpers/exception.h"

using namespace log4cxx;

//
// Enable logging in your class by DECLARing and INITing the logger
//
// class Foo : public Bar
// {
//  public:
//      DECLARE_LOGGER();
//      ...
// }
//
// INIT_LOGGER(FooClass, "Foo.Bar");
//
// Logging Macros are enabled by default. To remove the logging macros, 
// define DISABLE_LOGGING
//
//

#ifndef DISABLE_LOGGING

//
// Logging enabled
//
#define DECLARE_LOGGER()            static LoggerPtr logger
#define INIT_LOGGER(class,name)     LoggerPtr class::logger(Logger::getLogger(name))

#define LOG_TRACE(message, ...)     LOG4CXX_TRACE(this->logger, message ## __VA_ARGS__);
#define LOG_DEBUG(message, ...)     LOG4CXX_DEBUG(this->logger, message ## __VA_ARGS__);
#define LOG_INFO(message, ...)      LOG4CXX_INFO(this->logger, message ## __VA_ARGS__);
#define LOG_WARN(message, ...)      LOG4CXX_WARN(this->logger, message ## __VA_ARGS__);
#define LOG_ERROR(message, ...)     LOG4CXX_ERROR(this->logger, message ## __VA_ARGS__);
#define LOG_FATAL(message, ...)     LOG4CXX_FATAL(this->logger, message ## __VA_ARGS__);

#define LOG_ASSERT(condition, message, ...) LOG4CXX_ASSERT(this->logger, condition, message ## __VA_ARGS)

#define TRACE_ENTER()       LOG_TRACE("Enter: " << __PRETTY_FUNCTION__)
#define TRACE_EXIT()        LOG_TRACE("Exit: "  << __PRETTY_FUNCTION__)
#define TRACE_EXIT_RET(val) LOG_TRACE("Exit: "  << __PRETTY_FUNCTION__ << ": Returned --> " << val)

#else 

//
// Logging disabled 
//
#define DECLARE_LOGGER() 
#define INIT_LOGGER(class,name)
#define LOG_TRACE(message, ...)     
#define LOG_DEBUG(message, ...)     
#define LOG_INFO(message, ...)      
#define LOG_WARN(message, ...)      
#define LOG_ERROR(message, ...)     
#define LOG_FATAL(message, ...)    

#define TRACE_ENTER() 
#define TRACE_EXIT() 
#define TRACE_EXIT_RET(val)

#endif // DISABLE_LOGGING

#endif //  WATCHER_LOGGER_H
