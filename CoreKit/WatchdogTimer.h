/**
 * \file WatchdogTimer.h
 * \brief Definition of WatchdogTimer class
 * \date 2015-06-29
 * \author Ryan O'Farrell
 * \copyright National Aeronautics and Space Administration
 */

#ifndef WATCHDOGTIMER_H_
#define WATCHDOGTIMER_H_

//
// System Includes
//

//
// Library Includes
//

//
// Project Includes
//
#include "RunLoop.h"
#include "WatchdogExpiredCallback.h"
#include "AppLog.h"

namespace CoreKit
{

/**
 * \brief Watchdog timer class
 * Timer starts on activate and will call all callbacks unless deactivated before timeout reached.
 * \date 2015-06-19
 * \author Ryan O'Farrell
 */
class WatchdogTimer : public InterruptListener
{
public:
    RF_CK_FACTORY_COMPATIBLE(WatchdogTimer);
    /**
     * \brief Constructor
     * \param logLevel optional parameter to specify the log level for timer expired messages
     */
    WatchdogTimer(CoreKit::AppLog::Level logLevel = CoreKit::AppLog::LL_WARNING );

    /**
     * \brief Destructor
     */
    virtual ~WatchdogTimer();

    /**
     * \brief Initializes the watchdog timer with a run loop
     * \param runLoop the run loop to trigger the timer callback
     * \param interval The interval for ticks in seconds (defaults to 1 second)
     * \throws OsErrorException if problem registering timer in RunLoop
     */
    virtual void initialize(RunLoop * runLoop, float interval=1.0f);

    /**
     * \brief Terminates the Watchdog Timer instance by de-registering the timer
     * from the Run Loop
     * \throws OsErrorException if problem de-registering timer in RunLoop
     */
    virtual void terminate();

    /**
     * \brief Activates the watchdog timer
     * \param timeout time before timeout in ticks (based on timerExpired)
     */
    virtual void activate(uint32_t timeout);

    /**
     * \brief Deactivates the watchdog timer
     */
    virtual void deactivate();

    /**
     * \brief Resets the watchdog timer. Sets count to 0 but does not de-activate
     */
    virtual void reset();

    /*
     * \brief Gets the timer FD for this instance
     * \return timer FD for watchdog, -1 if not yet initialized
     */
    virtual int getTimerFd() const;

    /**
     * \brief Gets expiration time in number of ticks
     * \return expiration time
     */
    virtual uint32_t getTimeout() const;

    /**
     * \brief Registers a new expiration timer for when the watchdog expires
     * \param callback the instance to call
     */
    virtual void registerExpirationCallback(WatchdogExpiredCallback *callback);

    /**
     * \brief Callback for tick timer expired
     * \param timerFd timer that expired
     */
    virtual void timerExpired(int timerFd);

private:
    /**
     * \brief Private copy constructor
     * \param other object to copy
     */
    WatchdogTimer(WatchdogTimer const& other);

    /**
     * \brief Private assignment operator
     * \param other object to copy
     * \return reference to this object
     */
    WatchdogTimer& operator=(WatchdogTimer const& other);
    CoreKit::AppLog::Level m_logLevel;
    uint32_t m_counter;
    uint32_t m_timeout;
    bool m_active;
    std::vector<WatchdogExpiredCallback *> m_callbacks;
    int m_timerFd;
    RunLoop *m_runLoop;
};

} /* namespace GPP */
#endif /* WATCHDOGTIMER_H_ */
