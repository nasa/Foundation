/**
 * \file WatchdogExpiredCallback.h
 * \brief Definition of WatchdogExpiredCallback class
 * \date 2015-06-29
 * \author Ryan O'Farrell
 */

#ifndef WATCHDOGEXPIREDCALLBACK_H_
#define WATCHDOGEXPIREDCALLBACK_H_

namespace CoreKit
{

/**
 * \brief Interface class for watchdog timer callbacks
 * \date 2015-06-29
 * \author Ryan O'Farrell
 */
class WatchdogExpiredCallback
{
public:
    /**
     * \brief Constructor
     */
    WatchdogExpiredCallback();

    /**
     * \brief Destructor
     */
    virtual ~WatchdogExpiredCallback();

    /**
     * \brief callback when watchdog timer expires
     * \param timerFd file descriptor that identifies timer
     */
    virtual void operator()(int timerFd) = 0;
};

} /* namespace GPP */
#endif /* WATCHDOGEXPIREDCALLBACK_H_ */
