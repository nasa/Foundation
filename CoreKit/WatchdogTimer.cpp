/*
 * \file WatchdogTimer.cpp
 * \date 2015-06-29
 * \author Ryan O'Farrell
 */

#include <functional>
#include <algorithm>
#include "Application.h"
#include "WatchdogTimer.h"
#include "OsErrorException.h"

using std::for_each;
using std::bind2nd;
using std::mem_fun;

namespace CoreKit
{

WatchdogTimer::WatchdogTimer(CoreKit::AppLog::Level logLevel) :
        m_logLevel(logLevel), m_counter(0u), m_timeout(0u), m_active(false), m_callbacks(), m_timerFd(
                -1), m_runLoop(NULL)
{

}

static void deleteCallback(WatchdogExpiredCallback * callback)
{
    delete callback;
}

WatchdogTimer::~WatchdogTimer()
{
    if (m_timerFd > 0)
    {
        //need try/catch here to avoid exception being thrown in destructor
        try
        {
            terminate();
        } catch (OsErrorException const & exc)
        {
            if (NULL != G_MyApp)
            {
                G_MyApp->log() << AppLog::LL_ERROR
                        << "Watchdog timer: Exception while de-registering timer: "
                        << exc.what() << EndLog;
            }
        }
    }
    for_each(m_callbacks.begin(), m_callbacks.end(), deleteCallback);
}

void WatchdogTimer::terminate()
{
    if (NULL != m_runLoop)
    {
        if (m_timerFd > 0)
        {
            m_runLoop->deregisterTimer(m_timerFd);
            m_timerFd = -1;
        }
    }
}

void WatchdogTimer::initialize(CoreKit::RunLoop * runLoop, float interval)
{
    if (NULL != runLoop)
    {
        m_runLoop = runLoop;
        m_timerFd = m_runLoop->registerTimerWithInterval(interval, this, true);
    }
    else
    {
        if (NULL != G_MyApp)
        {
            G_MyApp->log() << AppLog::LL_ERROR
                    << "Watchdog timer: Can not register timer with NULL RunLoop instance"
                    << EndLog;
        }
    }

}

void WatchdogTimer::activate(uint32_t timeout)
{
    m_timeout = timeout;
    m_counter = 0;
    m_active = true;
}

void WatchdogTimer::deactivate()
{
    m_counter = 0;
    m_active = false;
}

void WatchdogTimer::reset()
{
    m_counter = 0;
}

int WatchdogTimer::getTimerFd() const
{
    return m_timerFd;
}

uint32_t WatchdogTimer::getTimeout() const
{
    return m_timeout;
}

void WatchdogTimer::registerExpirationCallback(
        WatchdogExpiredCallback *callback)
{
    if (NULL != callback)
    {
        m_callbacks.push_back(callback);
    }
    else
    {
        if (NULL != G_MyApp)
        {
            G_MyApp->log() << AppLog::LL_ERROR
                    << "Watchdog timer: Ignoring NULL callback register"
                    << EndLog;
        }

    }
}

void WatchdogTimer::timerExpired(int timerFd)
{
    if (m_active)
    {
        m_counter++;
        if (m_counter >= m_timeout)
        {
            if (NULL != G_MyApp)
            {
                G_MyApp->log() << m_logLevel
                        << "Watchdog timer: Timer expired after " << m_timeout
                        << " ticks.  Notifying listeners" << EndLog;
            }
            this->deactivate();
            for_each(m_callbacks.begin(), m_callbacks.end(),
                    bind2nd(mem_fun(&WatchdogExpiredCallback::operator()),
                            timerFd));
        }
    }
}

} /* namespace GPP */
