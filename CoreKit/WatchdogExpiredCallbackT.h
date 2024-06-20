/**
 * \file WatchdogExpiredCallbackT.h
 * \brief Definition of the WatchdogExpiredCallbackT Class
 * \date 2016-01-26
 * \author Ryan O'Farrell
 */

#ifndef WATCHDOGEXPIREDCALLBACKT_H_
#define WATCHDOGEXPIREDCALLBACKT_H_

#include "WatchdogExpiredCallback.h"

namespace CoreKit
{
	/**
	 * Defines a template class to support adapting callbacks for watchdog timer expiration
	 * \author Ryan O'Farrell
	 * \date 2016-01-26
	 */
	template<class TargetType>
	class WatchdogExpiredCallbackT : public WatchdogExpiredCallback
	{

	public:
	    /**
	     * \brief Constructor
	     * \param callbackTarget functor object to call
	     */
		explicit WatchdogExpiredCallbackT(TargetType callbackTarget)
		: m_callbackTarget(callbackTarget)
		{

		}

	    /**
	     * \brief Copy constructor
	     * \param other Instance to copy
	     */
		WatchdogExpiredCallbackT(WatchdogExpiredCallbackT const& other)
		: m_callbackTarget(other.m_callbackTarget)
		{

		}

		/**
		 * \brief Destructor
		 */
		virtual ~WatchdogExpiredCallbackT()
		{

		}

		/**
		 * \brief Operator interface for calling wrapped instance
		 * \param timerFd id of the watchdog timer that expired
		 */
		virtual void operator()(int timerFd)
		{
			m_callbackTarget(timerFd);
		}

	private:

		TargetType m_callbackTarget;
	};

	/**
	 * \brief Helper template function to create a new callback object.
	 * \details This function is useful for creating a new callback wrapper on the
	 * heap to be managed by a \c CoreKit::WatchdogTimer instance
	 * \param callbackTarget the target to wrap
	 */
	template<class TargetType>
	WatchdogExpiredCallback* newWatchdogExpiredCallback(TargetType callbackTarget)
	{
		return new WatchdogExpiredCallbackT<TargetType>(callbackTarget);
	}
}
#endif
