/**
 * \file TimerInputSource.h
 * \brief Definition of the TimerInputSource Class
 * \date 2012-09-24 08:39:58
 * \author Rolando J. Nieves
 */
#if !defined(EA_99058D7E_4524_4463_A96C_BE7F41C39E86__INCLUDED_)
#define EA_99058D7E_4524_4463_A96C_BE7F41C39E86__INCLUDED_

#include "InputSource.h"

namespace CoreKit
{
	/**
	 * \brief \c InputSource that Models One Shot and Recurring Timers
	 * 
	 * The \c TimerInputSource class makes it possible to integrate both one
	 * shot and recurring timers into a \c RunLoop instance.\par
	 *
	 * The \c TimerInputSource class utilizes the Linux \c timerfd() facility
	 * to create a file descriptor that exhibits input activity whenever a
	 * timer expires.
	 *
	 * \note
	 * Although the \c TimerInputSource class may be used on its own to
	 * register a timer with a \c RunLoop instance, the preferred way to
	 * register timers is using the \c RunLoop::registerTimer() method call.
	 *
	 * \author Rolando J. Nieves
	 * \date 2012-09-24 08:12:31
	 */
	class TimerInputSource : public CoreKit::InputSource
	{

	public:
		/**
		 * \brief Create the \c timerfd() Facility Used to Receive Timer Events
		 *
		 * The primary constructor for the \c TimerInputSource class creates
		 * the \c timerfd() facility used to receive timer events via a file
		 * descriptor.
		 *
		 * \param interval Period of the timer expressed in seconds as a
		 *                 decimal value.
		 * \param repeats \c true if the timer is recurring; \c false if it is
		 *                one-shot.
		 * \param timerListener \c InterruptListener instance interested in
		 *                      receiving events from this timer.
		 */
		TimerInputSource(double interval, bool repeats, InterruptListener *timerListener);
        /**
         * \brief Create the \c timerfd() Facility Using a Delayed First Callback
         *
         * The alternate constructor for the \c TimerInputSource class creates
         * the \c timerfd() facility initializes a recurring timer such that the
         * first callback is able to expire at a time dissimilar from the timer
         * period.
         *
         * \param firstTimeout Seconds to wait before the first callback, kicking
         *                     off the recurring timer.
         * \param interval Period of the timer expressed in seconds. Periodic
         *                 timer expirations only start after the first timeout
         *                 expires.
         * \param timerListener \c InterruptListener instance interested in
         *                      receiving events from this timer.
         */
        TimerInputSource(double firstTimeout, double interval, InterruptListener *timerListener);
		/**
		 * \brief Close the \c timerfd() Facility Used to Receive Timer Events
		 */
		virtual ~TimerInputSource();

		/**
		 * \brief Obtain the \c timerfd() File Descriptor Used to Receive Timer Events
		 *
		 * The \c fileDescriptor() method is part of the \c InputSource
		 * interface and is used primarily by the \c RunLoop class to obtain
		 * the file descriptor that will exhibit input activity whenever the
		 * timer modeled by this \c TimerInputSource instance expires.
		 *
		 * \return \c timerfd() file descriptor used to receive timer events.
		 */
		virtual int fileDescriptor() const;
		/**
		 * \brief Obtain the \c InterruptListener Interested in Timer Events
		 *
		 * The \c interruptListener() method is part of the \c InputSource
		 * interface and provides access to the \c InterruptListener instance
		 * that will receive timer events via the
		 * \c InterruptListener::timerExpired() method.
		 *
		 * \return \c InterruptListener interested in timer events from this
		 *         \c TimerInputSource instance.
		 */
		virtual InterruptListener* interruptListener() const;
		/**
		 * \brief Execute the \c InterruptListener::timerExpired() Callback
		 *
		 * The \c fireCallback() method is part of the \c InputSource interface
		 * and it calls the \c InterruptListener::timerExpired() method on
		 * the \c InterruptListener instance registered with this input source.
		 * \par
		 *
		 * This method is primarily used by the \c RunLoop class whenever it
		 * schedules this input source to perform work.
		 */
		virtual void fireCallback();

	private:
		/**
		 * \brief File Descriptor Used to Receive Timer Events
		 */
		int m_timerFd;
		/**
		 * \brief Decimal Number that Describes the Expiration Period for This Timer
		 */
		double m_timerInterval;
		/**
		 * \brief Flag that Indicates Whether this Timer is Recurring or One Shot
		 */
		bool m_repeats;
		/**
		 * \brief \c InterruptListener Derived Instance Interested in Timer Events from This Instance
		 */
		InterruptListener *m_timerListener;
	};

}
#endif // !defined(EA_99058D7E_4524_4463_A96C_BE7F41C39E86__INCLUDED_)
