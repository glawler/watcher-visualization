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

#undef LOG_DEBUG
#undef LOG_INFO
#undef LOG_ERROR
#ifdef WATCHER_LOGGER_H
    // borrowed from logger.h
    #define LOG_DEBUG(message, ...) { try{ LOG4CXX_DEBUG(logger, message ## __VA_ARGS__); } catch(std::exception &e) { std::cerr << "Exception in logging: " << e.what() << std::endl; } }
    #define LOG_INFO(message, ...) { try{ LOG4CXX_INFO(logger, message ## __VA_ARGS__); } catch(std::exception &e) { std::cerr << "Exception in logging: " << e.what() << std::endl; } }
    #define LOG_ERROR(message, ...) { try{ LOG4CXX_ERROR(logger, message ## __VA_ARGS__); } catch(std::exception &e) { std::cerr << "Exception in logging: " << e.what() << std::endl; } }
#endif
