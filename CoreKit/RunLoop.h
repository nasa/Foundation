/**
 * \file RunLoop.h
 * \brief Definition of the RunLoop Class
 * \date 2012-09-24 08:39:57
 * \author Rolando J. Nieves
 */
#if !defined(EA_FAF0C499_1C34_481d_B100_AF7997CA029B__INCLUDED_)
#define EA_FAF0C499_1C34_481d_B100_AF7997CA029B__INCLUDED_

#include <sys/epoll.h>
#include <vector>
#include <queue>
#include <map>

#include "InputSource.h"
#include "factory.h"
#include "TimerInputSource.h"

namespace CoreKit
{
class Thread;

	/**
	 * \brief Main Loop that Schedules All Work for a \c Thread
	 *
	 * The \c CoreKit::RunLoop class manages all of the input sources
	 * associated with an application and schedules work to service them.
	 * The \c CoreKit::RunLoop class does this by utilizing an operating
	 * system multiplexing service that is based on file descriptors.\par
	 *
	 * Along with generic input sources, \c CoreKit::RunLoop manages process
	 * signal listeners and application timers. Work to service those kind of
	 * interrupts is scheduled along with the generic input sources.\par
	 *
	 * All input sources that the \c CoreKit::RunLoop class manages must
	 * provide a file descriptor that exhibits input activity whenever the
	 * input source requires servicing. Typical external interfaces like serial
	 * ports and network sockets already behave in this way, thus integration
	 * into a \c CoreKit::RunLoop (via a \c CoreKit::InputSource derived class)
	 * is relatively easy to do.\par
	 *
	 * Other input sources that do not expose a file descriptor as defined
	 * above may implement a "monitoring" task or thread tied to the main
	 * thread via a simple inter-process communication conduit which does
	 * expose its input activity via a file descriptor (like \c eventfd() or
	 * named pipes).
	 *
	 * \author Rolando J. Nieves
	 * \date 2012-08-20 14:33:01
	 */
	class RunLoop
	{
		RF_CK_FACTORY_COMPATIBLE(RunLoop);
	public:

        /**
         * \brief Base class for end of loop iteration callback.
         */
		class LoopIterCbBase
		{
		public:
            /**
             * \brief Destructor.
             */
			virtual ~LoopIterCbBase() = default;

            /**
             * \brief Invoke callback.
             *
             * \param[in] theRunLoop - Pointer to the run loop whose iteration
             *            just finished.
             */
			virtual void operator()(RunLoop *theRunLoop) = 0;
		};

        /**
         * \brief Template class for adapting loop iteration callback.
         */
		template<class TargetType>
		class LoopIterCb : public LoopIterCbBase
		{
		public:
            /**
             * \brief Initialize callback with provided target.
             *
             * The target can be another callable, an instance of `std::bind()`,
             * or a lambda.
             *
             * \param[in] cbTarget - Target that will be called at the end of
             *                       the loop iteration.
             */
			inline LoopIterCb(TargetType cbTarget): m_cbTarget(cbTarget) {};

            /**
             * \brief Destructor.
             */
			virtual ~LoopIterCb() = default;

            /**
             * \brief Invoke target call.
             *
             * \param[in] theRunLoop - Pointer to the run loop whose iteration
             *            just finished.
             */
			virtual void operator()(RunLoop *theRunLoop) { m_cbTarget(theRunLoop); }

		private:
			TargetType m_cbTarget;
		};

        /**
         * \brief Factory facility for end-of-loop-iteration callbacks.
         *
         * \param[in] cbTarget - Target that will be called at the end of the
         *            loop iteration.
         *
         * \return Pointer to new end-of-loop-iteration callback instance.
         *         Management of the heap-allocated instance is the
         *         responsibility of the caller.
         */
		template<class TargetType>
		static LoopIterCbBase* newLoopIterCb(TargetType cbTarget)
		{ return new LoopIterCb<TargetType>(cbTarget); }

		/**
		 * \brief Initialize the Operating System Service Used for Input Multiplexing
		 *
		 * The primary constructor for the \c RunLoop class initializes the
		 * \c epoll() operating system service that is used to multiplex
		 * between all registered input sources. The multiplexing service is
		 * not activated, however, in this constructor.
		 *
		 */
		RunLoop();
		/**
		 * \brief Release All Resources Held by the \c RunLoop Instance
		 *
		 * The destructor for the \c RunLoop class closes down the \c epoll()
		 * operating system service used for input multiplexing. The
		 * destructor also deletes all signal and timer input sources created
		 * during the lifetime of this \c RunLoop instance.
		 */
		virtual ~RunLoop();

		/**
		 * \brief Obtain the \c Thread object that is hosting this \c RunLoop
		 *
		 * This method provides read-only access to the \c Thread object that
		 * is hosting this \c RunLoop. As this is not a required field to
		 * populate in the \c RunLoop class, this method may return \c NULL.
		 *
		 * \return \c Thread object hosting this \c RunLoop; may be \c NULL
		 */
		inline Thread* hostThread() const { return m_hostThread; }
		/**
		 * \brief Set the \c Thread object that is hosting this \c RunLoop
		 *
		 * This method provides modification access to the instance field that
		 * stores the \c Thread object hosting this \c RunLoop. Setting this
		 * field is not required for proper \c RunLoop operation.
		 *
		 * \param hostThread \c Thread object hosting this \c RunLoop.
		 */
		inline void setHostThread(Thread *hostThread) { m_hostThread = hostThread; }
		/**
		 * \brief Remove an Input Source from the Multiplexor
		 *
		 * The \c deregisterInputSource() method is used to remove an input
		 * source from the multiplexor \c RunLoop uses to schedule work. In
		 * other words, activity from the removed input source will no longer
		 * trigged work scheduling from this \c RunLoop instance.
		 *
		 * \param theInputSource \c InputSource instance that should be removed
		 *                       from this \c RunLoop instance's multiplexor.
		 */
		virtual void deregisterInputSource(InputSource* theInputSource);
		/**
		 * \brief Register a New Input Source in the Multiplexor
		 *
		 * The \c registerInputSource() method is used to add a new input
		 * source in the operating system multiplexor used by this \c RunLoop
		 * instance. After registration, activity from this new input source
		 * will trigger scheduling of work by this \c RunLoop instance.
		 *
		 * \param inputSource \c InputSource instance that should be added to
		 *                    this \c RunLoop instance's multiplexor.
         *
         * \note Although this method may be used to register signal and timer
         *       input sources, it is advised that users instead call
         *       \c registerSignalHandler() and \c registerTimerWithInterval()
         *       for this purpose, as the \c CoreKit::RunLoop class will attempt
         *       to manage the memory associated with those specialized input
         *       sources.
		 */
		virtual void registerInputSource(InputSource* inputSource);
		/**
		 * \brief Register an \c InterruptListener to Listen For Operating System Signals
		 *
		 * The \c registerSignalHandler() method is a simple way to register
		 * an \c InterruptListener that should be invoked whenever the
		 * application process receives the signal passed in the parameter
		 * list.\par
		 *
		 * Internally, this method simply creates an instance of the
		 * \c SignalInputSource class and registers it with the \c RunLoop
		 * instance multiplexor.
		 *
		 * \param signalNumber Integer identifying the signal that the provided
		 *                     \c InterruptListener derived instance is
		 *                     interested in. This parameter expects signal
		 *                     numbers as defined in the \c csignal header
		 *                     file.
		 * \param listener \c InterruptListener instance that is interested in
		 *                 the signal.
		 *
		 * \return Integer identifying the newly-registered \c SignalInputSource
		 */
		int registerSignalHandler(int signalNumber, InterruptListener* listener);
		/**
		 * \brief Initiate a Timer and Register it as an Input Source with this \c RunLoop
		 *
		 * The \c registerTimerWithInterval() method is an easy way to install
		 * a timer, and have expirations of this timer initiate input activity,
		 * forcing \c RunLoop to schedule work at every timer expiration.\par
		 *
		 * Internally, this method simply creates an instance of the
		 * \c TimerInputSource class and registers it with the multiplexor in
		 * this \c RunLoop instance.
		 *
		 * \param timeInterval Interval between timer expirations, expressed
		 *                     in seconds as a floating point value.
		 * \param theListener \c InterruptListener instance that is interested
		 *                    in the expirations of this timer.
		 * \param repeats \c true if this is a recurring timer; \c false
		 *                otherwise.
		 *
		 * \return Integer identifying the newly-registered \c TimerInputSource
		 */
		int registerTimerWithInterval(double timeInterval, InterruptListener* theListener, bool repeats);
        /**
         * \brief Initiate a Recurring Timer with a delayed start and register it as an Input Source with this \c RunLoop
         *
         * This \c registerTimerWithInterval() overload gives the application
         * developer the ability to register a recurring timer that starts after
         * a set amount of time, not necessarily the same as the recurring period,
         * expires.
         *
         * Similar to the primary \c registerTimerWithInterva(double,InterruptListener*,bool)
         * version, this method simply creates an instance of the \c TimerInputSource
         * class and registers it with the multiplexor in this \c RunLoop
         * instance.
         *
         * \param firstTimeout Time to wait before the recurring timer starts,
         *                     expressed in seconds as a floating point value.
         *                     A callback will occur once this amount of time
         *                     elapses
         * \param timeInterval Interval between timer expirations, expressed in
         *                     seconds as a floating point value.
         * \param theListener \c InterruptListener instance that is interested
         *                    in the expirations of this timer.
         *
         * \return Identifier for the newly-registered \c TimerInputSource
         */
        int registerTimerWithInterval(double firstTimeout, double timeInterval, InterruptListener *theListener);
		/**
		 * \brief Deregister a timer from the Multiplexor
		 *
		 * The \c deregisterTimer() method is used to remove a timer input source
		 * that was previously created using the \c registerTimerWithInterval()
		 * method.
		 *
		 * \param timerId Timer identifier offered by the \c 
		 *                registerTimerWithInterval() method.
		 */
		virtual void deregisterTimer(int timerId);

        /**
         * \brief Register and end-of-loop-iteration callback.
         *
         * The callback provided is invoked every time this class completes
         * a run loop iteration. Once submitted to this method, the
         * \c CoreKit::RunLoop class assumes ownership of the instance memory
         * and will de-allocate it once this instance is destroyed.
         *
         * \param[in] loopIterEndCb - Object to invoke at every run loop
         *            iteration end.
         */
		void addLoopIterEndCallback(LoopIterCbBase *loopIterEndCb);
		/**
		 * \brief Monitor All Input Sources and Schedule Work Accordingly
		 *
		 * The \c run() method is where a framework application's calling
		 * \c Thread object surrenders control to the work scheduler hosted in
		 * this \c RunLoop instance. This method does not return to the caller
		 * until the \c RunLoop instance has been instructed to terminate.
		 */
		virtual void run();
		/**
		 * \brief Request Termination of this \c RunLoop Work Scheduler
		 *
		 * The \c terminate() method instructs this \c RunLoop instance's
		 * work scheduler to terminate at its next available opportunity.
		 */
		inline void terminate() { m_terminationRequested = true; }
		/**
		 * \brief Check Whether Termination of this \c RunLoop Work Scheduler was Requested
		 *
		 * The \c isTerminationRequested() method informs the caller if this
		 * \c RunLoop instance has been asked to terminate its work scheduler.
		 *
		 * \return \c true if this \c RunLoop instance's work scheduler has
		 *         been asked to terminate; \c false otherwise.
		 */
		inline bool isTerminationRequested() const { return m_terminationRequested; }

	protected:
		struct InputSourceSort
		{
		public:
			bool operator()(InputSource *left, InputSource *right);
		};

		void pushEpollEventInputSource(struct epoll_event anEvent);
		/**
		 * \brief Collection of All \c InputSource Instances Managed by this \c RunLoop Instance
		 */
		std::vector<InputSource*> m_inputSources;
		/**
		 * \brief Dictionary holding all timers registered with this \c RunLoop instance
		 */
		std::map<int, TimerInputSource*> m_timers;
		/**
		 * \brief Handle to the Operating System Input Multiplexing Service
		 */
		int m_epollFd;
		/**
		 * \brief Field Used to Remember if Work Scheduler Termination Was Requested.
		 */
		bool m_terminationRequested;
		std::vector<LoopIterCbBase*> m_loopIterEndCb;
		typedef std::priority_queue<InputSource*,std::vector<InputSource*>,InputSourceSort> ActivityQueue_t;
		ActivityQueue_t m_sortedActivityQueue;
		Thread *m_hostThread;

		void fireEndOfLoopCbs();
	};

}
#endif // !defined(EA_FAF0C499_1C34_481d_B100_AF7997CA029B__INCLUDED_)
