/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 * 
 *     You should have received a copy of the GNU General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef WATCHER_LOGGER_H
#define WATCHER_LOGGER_H

#include <iostream>

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/exception.h>

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

// The one and only global logger for non-class-functions
// Initialized in logger.cpp. Used as a global scope instance
// of 'logger' in the macros below.
extern LoggerPtr logger;

//
// Logging enabled
//
#define DECLARE_LOGGER()            static LoggerPtr logger
#define INIT_LOGGER(class,name)     LoggerPtr class::logger(Logger::getLogger(name))

// Log4cxx is crashing, so these are wrapped in try/catch blocks. Which is really annoying. 
#define LOG_TRACE(message, ...)     do { try{ LOG4CXX_TRACE(logger, message ## __VA_ARGS__); } catch(std::exception &e) { std::cerr << "Exception in logging: " << e.what() << std::endl; } } while(0)
#define LOG_DEBUG(message, ...)     do { try{ LOG4CXX_DEBUG(logger, message ## __VA_ARGS__); } catch(std::exception &e) { std::cerr << "Exception in logging: " << e.what() << std::endl; } } while(0)
#define LOG_INFO(message, ...)      do { try{ LOG4CXX_INFO(logger, message ## __VA_ARGS__); } catch(std::exception &e) { std::cerr << "Exception in logging: " << e.what() << std::endl; } } while(0) 
#define LOG_WARN(message, ...)      do { try{ LOG4CXX_WARN(logger, message ## __VA_ARGS__); } catch(std::exception &e) { std::cerr << "Exception in logging: " << e.what() << std::endl; } } while(0)
#define LOG_ERROR(message, ...)     do { try{ LOG4CXX_ERROR(logger, message ## __VA_ARGS__); } catch(std::exception &e) { std::cerr << "Exception in logging: " << e.what() << std::endl; } } while(0)
#define LOG_FATAL(message, ...)     do { try{ LOG4CXX_FATAL(logger, message ## __VA_ARGS__); } catch(std::exception &e) { std::cerr << "Exception in logging: " << e.what() << std::endl; } } while(0)

#define LOG_ASSERT(condition, message, ...) LOG4CXX_ASSERT(this->logger, condition, message ## __VA_ARGS)

#define TRACE_ENTER()       do { try{ LOG_TRACE("Enter: " << __PRETTY_FUNCTION__); } catch(std::exception &e) { std::cerr << "Exception in logging: " << e.what() << std::endl; } } while(0)
#define TRACE_EXIT()        do { try{ LOG_TRACE("Exit: "  << __PRETTY_FUNCTION__); } catch(std::exception &e) { std::cerr << "Exception in logging: " << e.what() << std::endl; } } while(0)
#define TRACE_EXIT_RET(val) do { try{ LOG_TRACE("Exit: "  << __PRETTY_FUNCTION__ << ": Returned --> " << val); } catch(std::exception &e) { std::cerr << "Exception in logging: " << e.what() << std::endl; } } while(0)
#define TRACE_EXIT_RET_BOOL(val) do { try{ LOG_TRACE("Exit: "  << __PRETTY_FUNCTION__ << ": Returned --> " << (val ? "true":"false")); } catch(std::exception &e) { std::cerr << "Exception in logging: " << e.what() << std::endl; } } while(0)

#define LOAD_LOG_PROPS(file) { try{ log4cxx::PropertyConfigurator::configure(file); } catch(std::exception &e) { std::cerr << "Exception while loding log properties: " << e.what() << std::endl; } }

#else 

//
// Logging disabled 
//
#define DECLARE_LOGGER() static LoggerPtr logger
#define INIT_LOGGER(class,name) LoggerPtr class::logger(Logger::getLogger(name))
#define LOG_TRACE(message, ...) do { } while(0)
#define LOG_DEBUG(message, ...) do { } while(0)
#define LOG_INFO(message, ...) do { } while(0)
#define LOG_WARN(message, ...) do { } while(0)
#define LOG_ERROR(message, ...) do { } while(0)
#define LOG_FATAL(message, ...) do { } while(0)

#define TRACE_ENTER()  do { } while(0)
#define TRACE_EXIT()  do { } while(0)
#define TRACE_EXIT_RET(val) do { } while(0)
#define TRACE_EXIT_RET_BOOL(val) do { } while(0)

#define LOAD_LOG_PROPS(file) 

#endif // DISABLE_LOGGING

#endif //  WATCHER_LOGGER_H
