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

/**@file
 * @author Michael Elkins <Michael.Elkins@cobham.com>
 * @date 2009-04-24
 */

#include <pthread.h>
//#include <iostream>
#include <unistd.h>

#if 0
namespace
{
    pthread_mutex_t busy_lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t busy_cond = PTHREAD_COND_INITIALIZER;
}
#endif

extern "C" int busy_handler(void *, int)
{
    //std::cout << "busy_handler: enter (thread " << pthread_self() << ")\n";
#if 0
    pthread_mutex_lock(&busy_lock);
    pthread_cond_wait(&busy_cond, &busy_lock);
    pthread_mutex_unlock(&busy_lock);
#else

#define BUSY_SLEEP 20 /* milliseconds */
    usleep(BUSY_SLEEP);
#endif

    //std::cout << "busy_handler: exit (thread " << pthread_self() << ")\n";

    return 1; // nonzero means try again
}

void busy_done()
{
    //std::cout << "busy_done: enter (thread " << pthread_self() << ")\n";
    //pthread_cond_signal(&busy_cond);
    //std::cout << "busy_done: exit (thread " << pthread_self() << ")\n";
}
