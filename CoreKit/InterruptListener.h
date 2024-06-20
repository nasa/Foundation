/**
 * \file InterruptListener.h
 * \brief Definition of the InterruptListener Class
 * \date 2012-09-24 08:39:57
 * \author Rolando J. Nieves
 */
#if !defined(EA_C0ED3862_AF69_4d0b_8DE4_3E456E13387B__INCLUDED_)
#define EA_C0ED3862_AF69_4d0b_8DE4_3E456E13387B__INCLUDED_

namespace CoreKit
{
	class InputSource;

	/**
	 * \brief Interface for Classes that Service \c RunLoop Interrupts.
	 *
	 * The \c CoreKit::InterruptListener class defines the interface that other
	 * classes must abide to in order to receive and service interrupts
	 * generated from a \c Thread instance's \c RunLoop. \par
	 *
	 * There are three kinds of interrupts that a \c RunLoop may generate:
	 * - Process Signals (e.g., \c SIGTERM, \c SIGINT )
	 * - Timers
	 * - Generic Input Activity
	 *
	 * An \c InterruptHandler instance is usually submitted to a \c RunLoop
	 * in association with a specific interrupt source. For example, whenever
	 * a timer is registered with the \c RunLoop, an \c InterruptListener
	 * derived instance must accompany the registration. It is that
	 * \c InterruptListener derived instance that will receive the callback
	 * whenever the registered timer expires.
	 *
	 * \note
	 * \c InterruptListener instances are not exclusively bound to a specific
	 * interrupt. The same instance may be re-used with several interrupt
	 * input sources.
	 *
	 * \author Rolando J. Nieves
	 * \date 21-Aug-2012 3:45:11 PM
	 */
	class InterruptListener
	{

	public:
		/**
		 * \brief Default constructor for \c InterruptListener
		 */
		InterruptListener();
		/**
		 * \brief End of Destructor Chain for \c InterruptListener Derived Classes
		 */
		virtual ~InterruptListener();

		/**
		 * \brief Interrupt Handling Method for Generic Input Activity.
		 *
		 * The \c inputAvailableFrom() method is called on any
		 * \c InterruptHandler derived instance whenever there is input
		 * activity on an associated generic input source.
		 *
		 * \param theInputSource \c InputSource that generated the input
		 *                       activity.
		 */
		virtual void inputAvailableFrom(InputSource* theInputSource);
		/**
		 * \brief Interrupt Handling Method for Process Signals.
		 *
		 * An \c InterruptListener instance that is registered to receive
		 * interrupt calls from signals sent to the application process would
		 * have its \c signalReceived() method called.
		 *
		 * \param signalNumber Integer identifying the signal received by the
		 *                     application process.
		 */
		virtual void signalReceived(int signalNumber);
		/**
		 * \brief Interrupt Handling Method for Timers
		 *
		 * Whenever a \c RunLoop timer associated with this \c InterruptHandler
		 * derived instance expires, that instance would have its
		 * \c timerExpired() method called.
		 *
		 * \param timerId Integer identifying the timer that expired.
		 */
		virtual void timerExpired(int timerId);

	};

}
#endif // !defined(EA_C0ED3862_AF69_4d0b_8DE4_3E456E13387B__INCLUDED_)
