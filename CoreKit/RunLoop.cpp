/**
 * \file RunLoop.cpp
 * \brief Implementation of the RunLoop Class
 * \date 2012-09-24 08:39:57
 * \author Rolando J. Nieves
 */

#include <unistd.h>
#include <sys/epoll.h>
#include <errno.h>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <functional>

#include "RunLoop.h"
#include "Thread.h"
#include "SignalInputSource.h"
#include "OsErrorException.h"

#define RF_RL_MAX_SIMULT_EVENTS (10u)
#define RF_RL_EPOLL_TIMEOUT (1000u)

using std::find;
using std::for_each;
using std::mem_fun;
using std::ptr_fun;
using std::priority_queue;
using std::bind1st;
using std::map;
using CoreKit::RunLoop;
using CoreKit::InterruptListener;
using CoreKit::SignalInputSource;
using CoreKit::TimerInputSource;
using CoreKit::InputSource;
using CoreKit::OsErrorException;

/**
 * \brief Delete an \c InputSource instance if It Represents a Timer or Signal
 *
 * The \c deleteSignalAndTimerIs() local function deletes any \c InputSource
 * instances passed to it as long as \c SignalInputSource or
 * \c TimerInputSource is part of their class inheritance lineage. This
 * function is primarily used during cleanup in a \c std::for_each loop.
 *
 * \param anInputSource \c InputSource instance to delete if the aforementioned
 *                      conditions are met.
 */
static void deleteSignalAndTimerIs(InputSource *anInputSource)
{
    SignalInputSource *theSignalIs = NULL;
    TimerInputSource *theTimerIs = NULL;

    /*
     * Use dynamic_cast<> to test the lineage of the instance passed as input.
     * dynamic_cast<> will return NULL if the inheritance lineage test fails.
     */
    if ((theSignalIs = dynamic_cast<SignalInputSource*>(anInputSource)) != NULL)
    {
        delete theSignalIs;
    }
    else if ((theTimerIs = dynamic_cast<TimerInputSource*>(anInputSource)) != NULL)
    {
        delete theTimerIs;
    }
}


static void deleteLoopIterCb(RunLoop::LoopIterCbBase *aLoopIterCb)
{
    delete aLoopIterCb;
}


RunLoop::RunLoop()
: m_epollFd(-1), m_terminationRequested(false), m_sortedActivityQueue(RunLoop::InputSourceSort()),
  m_hostThread(NULL)
{
    m_epollFd = epoll_create(RF_RL_MAX_SIMULT_EVENTS);
    if (-1 == m_epollFd)
    {
        throw OsErrorException("epoll_create", errno);
    }
}


RunLoop::~RunLoop()
{
    if (m_epollFd != -1)
    {
        close(m_epollFd);
    }

    /*
     * We only delete timer and signal input sources, because we do not own
     * the other generic input sources.
     */
    for_each(m_inputSources.begin(), m_inputSources.end(), ptr_fun(&deleteSignalAndTimerIs));
    m_inputSources.clear();

    for_each(m_loopIterEndCb.begin(), m_loopIterEndCb.end(), ptr_fun(&deleteLoopIterCb));
    m_loopIterEndCb.clear();
}


void RunLoop::deregisterInputSource(InputSource* theInputSource)
{
    auto isIter = find(
        m_inputSources.begin(),
        m_inputSources.end(),
        theInputSource
    );

    if (isIter != m_inputSources.end())
    {
        m_inputSources.erase(isIter);
        struct epoll_event anEvent;

        // Initialize the epoll() event instance that will be used to remove
        // the input source's file descriptor with our epoll() service
        // instance.
        memset(&anEvent, 0x00, sizeof(anEvent));
        anEvent.events = EPOLLIN;
        anEvent.data.ptr = theInputSource;

        if
        (
            epoll_ctl(
                m_epollFd,
                EPOLL_CTL_DEL,
                theInputSource->fileDescriptor(),
                &anEvent
            ) == -1
        )
        {
            throw OsErrorException("epoll_ctl", errno);
        }
    }
}


void RunLoop::registerInputSource(InputSource* inputSource)
{
    struct epoll_event anEvent;

    if (nullptr == inputSource)
    {
        return;
    }

    /*
     * Initialize the epoll() event instance that will be used to register the
     * new input source's file descriptor with our epoll() service instance.
     * In doing so, we're storing a pointer to the input source in the epoll
     * event structure so that when this input source exhibits activity we can
     * quickly schedule their work.
     */
    memset(&anEvent, 0x00, sizeof(anEvent));
    anEvent.events = EPOLLIN;
    anEvent.data.ptr = inputSource;

    if (epoll_ctl(m_epollFd, EPOLL_CTL_ADD, inputSource->fileDescriptor(), &anEvent) == -1)
    {
        throw OsErrorException("epoll_ctl", errno);
    }

    m_inputSources.push_back(inputSource);
}


int RunLoop::registerSignalHandler(int signalNumber, InterruptListener* listener)
{
    SignalInputSource *aSignalIs = NULL;

    aSignalIs = new SignalInputSource(signalNumber, listener);
    this->registerInputSource(aSignalIs);

    return aSignalIs->fileDescriptor();
}


int RunLoop::registerTimerWithInterval(double timeInterval, InterruptListener* theListener, bool repeats)
{
    TimerInputSource *aTimerIs = NULL;

    aTimerIs = new TimerInputSource(timeInterval, repeats, theListener);
    this->registerInputSource(aTimerIs);
    m_timers[aTimerIs->fileDescriptor()] = aTimerIs;

    return aTimerIs->fileDescriptor();
}


int RunLoop::registerTimerWithInterval(double firstTimeout, double timeInterval, InterruptListener *theListener)
{
    TimerInputSource *aTimerIs = NULL;
    
    aTimerIs = new TimerInputSource(firstTimeout, timeInterval, theListener);
    this->registerInputSource(aTimerIs);
    m_timers[aTimerIs->fileDescriptor()] = aTimerIs;
    
    return aTimerIs->fileDescriptor();
}


void RunLoop::deregisterTimer(int timerId)
{
    map<int, TimerInputSource*>::iterator timerEntry;
    
    timerEntry = m_timers.find(timerId);
    if (timerEntry != m_timers.end())
    {
        deregisterInputSource((*timerEntry).second);
        TimerInputSource *to_delete = (*timerEntry).second;
        m_timers.erase(timerEntry);
        delete to_delete;
        to_delete = nullptr;
    }
}


void RunLoop::addLoopIterEndCallback(RunLoop::LoopIterCbBase *loopIterEndCb)
{
    m_loopIterEndCb.push_back(loopIterEndCb);
}


void RunLoop::run()
{
    struct epoll_event theEvents[RF_RL_MAX_SIMULT_EVENTS];
    int numFds = 0;
    InputSource *anInputSource = NULL;

    /*
     * This loop continues to run until a termination is requested as evident
     * by the value of the m_terminationRequested instance field.
     */
    do
    {
        numFds = epoll_wait(m_epollFd, &theEvents[0], RF_RL_MAX_SIMULT_EVENTS, RF_RL_EPOLL_TIMEOUT);
        if (numFds > 0)
        {
            /*
             * Store all the events that epoll_wait() detected into a
             * prioritized queue. Then process each event from highest priority
             * to lowest.
             */
            for_each(&theEvents[0], &theEvents[numFds], bind1st(mem_fun(&RunLoop::pushEpollEventInputSource), this));
            while (!m_terminationRequested && (m_sortedActivityQueue.size() > 0))
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

        /*
         * Invoke the end-of-loop list of callbacks.
         */
        if (!m_terminationRequested)
        {
            this->fireEndOfLoopCbs();
        }
    } while (false == m_terminationRequested);
}


bool RunLoop::InputSourceSort::operator()(InputSource *left, InputSource *right)
{
    bool result = true;

    /*
     * Lower relative priority values translate to a higher priority.
     */
    if (left->relativePriority() < right->relativePriority())
    {
        result = false;
    }

    return result;
}


void RunLoop::fireEndOfLoopCbs()
{
    for_each(m_loopIterEndCb.begin(), m_loopIterEndCb.end(), bind2nd(mem_fun(&LoopIterCbBase::operator()), this));
}


void RunLoop::pushEpollEventInputSource(struct epoll_event anEvent)
{
    m_sortedActivityQueue.push(reinterpret_cast<InputSource*>(anEvent.data.ptr));
}

// vim: set ts=4 sw=4 expandtab:
