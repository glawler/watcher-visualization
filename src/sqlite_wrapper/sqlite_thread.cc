/**@file
 * @author Michael Elkins <Michael.Elkins@cobham.com>
 * @date 2009-04-24
 */

#include <pthread.h>

namespace
{
    pthread_mutex_t busy_lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t busy_cond = PTHREAD_COND_INITIALIZER;
}

extern "C" int busy_handler(void *, int)
{
    pthread_mutex_lock(&busy_lock);
    pthread_cond_wait(&busy_cond, &busy_lock);
    pthread_mutex_unlock(&busy_lock);

    return 1; // nonzero means try again
}

void busy_done()
{
    pthread_cond_signal(&busy_cond);
}
