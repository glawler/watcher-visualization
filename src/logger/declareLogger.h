/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Affero General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Affero General Public License for more details.
 * 
 *     You should have received a copy of the GNU Affero General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef IM_A_LOGGER_AND_IM_OK_H_I_SLEEP_ALL_NIGHT_AND_I_WORK_ALL_DAY_H
#define IM_A_LOGGER_AND_IM_OK_H_I_SLEEP_ALL_NIGHT_AND_I_WORK_ALL_DAY_H

/* @file declareLogger.h
 * Include this file in your module that uses the logger. It allows
 * you to declare an instance of logger without pulling in macros that 
 * other modules may not want. Then include logger.h in your .cpp file. 
 * This means that you cannot use the LOG_ macros in your header files though.
 * So, err, sorry about that. 
 */

#ifndef DISABLE_LOGGING
    // Use these two to declare module (class level or namespace level) loggers. 
    #define DECLARE_LOGGER()            static const char *loggerName; 
    #define INIT_LOGGER(class,name)     const char *class::loggerName=name

    // Use this to declare a global logger. There can be onlu one global logger, be careful.
    #define DECLARE_GLOBAL_LOGGER(name) static  const char *loggerName=name

#else
    #define DECLARE_LOGGER() 
    #define INIT_LOGGER(class,name)     

    #define DECLARE_GLOBAL_LOGGER(name) 
#endif

#endif /* IM_A_LOGGER_AND_IM_OK_H_I_SLEEP_ALL_NIGHT_AND_I_WORK_ALL_DAY_H */
