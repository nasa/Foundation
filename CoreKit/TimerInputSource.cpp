/**
 * \file TimerInputSource.cpp
 * \brief Implementation of the TimerInputSource Class
 * \date 2012-09-24 08:39:58
 * \author Rolando J. Nieves
 */

#include <unistd.h>
#include <fcntl.h>
#include <sys/timerfd.h>
#include <stdint.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cerrno>

#include <CoreKit/InterruptListener.h>
#include <CoreKit/OsErrorException.h>
#include <CoreKit/SystemTime.h>

#include "TimerInputSource.h"

using CoreKit::TimerInputSource;
using CoreKit::InterruptListener;
using CoreKit::OsErrorException;

TimerInputSource::TimerInputSource(double interval, bool repeats, InterruptListener *timerListener)
: m_timerInterval(interval), m_repeats(repeats), m_timerFd(-1), m_timerListener(timerListener)
{
    struct itimerspec timerSpec;

    if (nullptr == timerListener)
    {
        throw std::runtime_error("Invalid interrupt listener dependency injected.");
    }

    memset(&timerSpec, 0x00, sizeof(timerSpec));
    SystemTime::secsToTimespec(interval, timerSpec.it_value);
    if (m_repeats)
    {
        SystemTime::secsToTimespec(interval, timerSpec.it_interval);
    }
    m_timerFd = timerfd_create(CLOCK_REALTIME,
#if defined(HAVE_TFD_NONBLOCK) && (HAVE_TFD_NONBLOCK == 1)
        TFD_NONBLOCK
#else
        0
#endif /* defined(HAVE_TFD_NONBLOCK) && (HAVE_TFD_NONBLOCK == 1) */
        );
    if (-1 == m_timerFd)
    {
        throw OsErrorException("timerfd_create", errno);
    }
#if !defined(HAVE_TFD_NONBLOCK) || (HAVE_TFD_NONBLOCK == 0)
    if (fcntl(m_timerFd, F_SETFL, O_NONBLOCK) == -1)
    {
        throw OsErrorException("timer fd fcntl", errno);
    }
#endif /* !defined(HAVE_TFD_NONBLOCK) || (HAVE_TFD_NONBLOCK == 0) */

    if (timerfd_settime(m_timerFd, 0, &timerSpec, nullptr) == -1)
    {
        throw OsErrorException("timerfd_settime", errno);
    }
}


TimerInputSource::TimerInputSource(double firstTimeout, double interval, InterruptListener *timerListener)
: m_timerInterval(interval), m_repeats(true), m_timerFd(-1), m_timerListener(timerListener)
{
    struct itimerspec timerSpec;
    
    if (nullptr == timerListener)
    {
        throw std::runtime_error("Invalid interrupt listener dependency injected.");
    }
    
    memset(&timerSpec, 0x00, sizeof(timerSpec));
    SystemTime::secsToTimespec(firstTimeout, timerSpec.it_value);
    SystemTime::secsToTimespec(interval, timerSpec.it_interval);
    m_timerFd = timerfd_create(CLOCK_REALTIME,
#if defined(HAVE_TFD_NONBLOCK) && (HAVE_TFD_NONBLOCK == 1)
        TFD_NONBLOCK
#else
        0
#endif /* defined(HAVE_TFD_NONBLOCK) && (HAVE_TFD_NONBLOCK == 1) */
    );
    if (-1 == m_timerFd)
    {
        throw OsErrorException("TimerInputSource alternate constructor timerfd_create", errno);
    }
#if !defined(HAVE_TFD_NONBLOCK) || (HAVE_TFD_NONBLOCK == 0)
    if (fcntl(m_timerFd, F_SETFL, O_NONBLOCK) == -1)
    {
        throw OsErrorException("TimerInputSource alternate constructor timer fd fcntl", errno);
    }
#endif /* !defined(HAVE_TFD_NONBLOCK) || (HAVE_TFD_NONBLOCK == 0) */

    if (timerfd_settime(m_timerFd, 0, &timerSpec, nullptr) == -1)
    {
        throw OsErrorException("TimerInputSource alternate constructor timerfd_settime", errno);
    }
}


TimerInputSource::~TimerInputSource()
{
    if (m_timerFd != -1)
    {
        close(m_timerFd);
    }
}





int TimerInputSource::fileDescriptor() const
{
    return m_timerFd;
}


InterruptListener* TimerInputSource::interruptListener() const
{
    return  m_timerListener;
}


void TimerInputSource::fireCallback()
{
    ssize_t readResult = 0;
    uint64_t readVal = 0uLL;

    do
    {
        /*
         * Upon timer expiration, the timerfd() facility transmits a 64-bit
         * integer value via its associated file descriptor. See the manual
         * page for timerfd_create() for more information.
         */
        readResult = read(m_timerFd, &readVal, sizeof(readVal));
    } while (sizeof(readVal) == readResult);

    m_timerListener->timerExpired(m_timerFd);
}

// vim: set ts=4 sw=4 expandtab:
