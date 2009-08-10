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

#ifdef DELTA_LOG
    // borrowed from dtUtil/log.h
    #define LOG_DEBUG(msg)\
        dtUtil::Log::GetInstance().LogMessage(__FUNCTION__, __LINE__, msg,dtUtil::Log::LOG_DEBUG);
    #define LOG_INFO(msg)\
        dtUtil::Log::GetInstance().LogMessage(__FUNCTION__, __LINE__, msg,dtUtil::Log::LOG_INFO);
    #define LOG_ERROR(msg)\
        dtUtil::Log::GetInstance().LogMessage(__FUNCTION__, __LINE__, msg,dtUtil::Log::LOG_ERROR);
#endif
