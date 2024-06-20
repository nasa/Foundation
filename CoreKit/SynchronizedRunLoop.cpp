/**
 * \file SynchronizedRunLoop.cpp
 * \brief Implementation of the SynchronizedRunLoop Class
 * \date 2012-11-16 10:51:20
 * \author Rolando J. Nieves
 */

#include <sys/epoll.h>
#include <cerrno>
#include <algorithm>
#include <functional>


#include <CoreKit/OsErrorException.h>
#include <CoreKit/InputSource.h>

#include "SynchronizedRunLoop.h"

#define RF_SRL_MAX_SIMULT_EVENTS (50u)

using std::for_each;
using std::bind1st;
using std::mem_fun;
using CoreKit::SynchronizedRunLoop;
using CoreKit::RunLoop;
using CoreKit::OsErrorException;
using CoreKit::InputSource;

SynchronizedRunLoop::SynchronizedRunLoop(sem_t* runLoopSyncObj)
: RunLoop(), m_runLoopSyncObj(runLoopSyncObj)
{

}





void SynchronizedRunLoop::run()
{
    int waitResult = 0;
    int numFds = 0;
    struct epoll_event theEvents[RF_SRL_MAX_SIMULT_EVENTS];
    InputSource *anInputSource = NULL;

    /*
     * At the top of this loop we have a semaphore wait, that will force us to
     * remain dormant until the external scheduler "posts" (i.e., increments)
     * the semaphore, indicating it's our turn to run. After waiting on the
     * semaphore, this loop examines all input sources and schedules work for
     * those that exhibit input activity. The examination of these input
     * sources (via epoll_wait()) doesn't block but instead reports what file
     * descriptors are ready for input servicing.
     */
    do
    {
        /*
         * Wait for the semaphore to be available. That means it's our turn.
         */
        waitResult = sem_wait(m_runLoopSyncObj);
        if ((waitResult != 0) && (errno != EINTR))
        {
            throw OsErrorException("sem_wait", errno);
        }

        /*
         * The timeout for this epoll_wait() call is 0, meaning it will not
         * block and simply return with the number of file descriptors that are
         * exhibiting input activity at the time of the call.
         */
        numFds = epoll_wait(m_epollFd, &theEvents[0], RF_SRL_MAX_SIMULT_EVENTS,0);
        if (numFds > 0)
        {
            /*
             * Store all the events that epoll_wait() detected into a
             * prioritized queue. Then process each event from highest priority
             * to lowest.
             */
            for_each(&theEvents[0], &theEvents[numFds], bind1st(mem_fun(&SynchronizedRunLoop::pushEpollEventInputSource), this));
            while (m_sortedActivityQueue.size() > 0)
            {
                /*
                 * The InputSource instance pointer is stored in the event
                 * definition, so in order to schedule the work for this source
                 * all we have to do is typecast the "ptr" union member of the
                 * "data" event structure member, and call its "fireCallback()"
                 * method. The InputSource will know what to do from there.
                 */
                anInputSource = m_sortedActivityQueue.top();
                anInputSource->fireCallback();
                m_sortedActivityQueue.pop();
            }
        }


        if (!m_terminationRequested)
        {
            this->fireEndOfLoopCbs();
        }
    } while(false == m_terminationRequested);
}

// vim: set ts=4 sw=4 expandtab:
