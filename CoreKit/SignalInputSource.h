/**
 * \file SignalInputSource.h
 * \brief Definition of the SignalInputSource Class
 * \date 2012-09-24 08:39:57
 * \author Rolando J. Nieves
 */
#if !defined(EA_66751973_5C51_4baf_B560_3271FEE3738E__INCLUDED_)
#define EA_66751973_5C51_4baf_B560_3271FEE3738E__INCLUDED_

#include "InputSource.h"

namespace CoreKit
{
	/**
	 * \brief \c InputSource Derivative Used to Receive Process Signals
	 *
	 * The \c SignalInputSource class makes it possible to integrate the
	 * reception of operating system signals within a \c Thread \c RunLoop.
	 * \par
	 * 
	 * The \c SignalInputSource class utilizes the Linux \c signalfd()
	 * facilities to create a file descriptor that exhibits input activity
	 * whenever the host process receives a signal from the operating system.
	 *
	 * \note
	 * Although the \c SignalInputSource class may be used on its own to
	 * register a signal handler with a \c Thread \c RunLoop, the preferred way
	 * to register for the reception of signals in a process is via the
	 * \c RunLoop::registerSignalHandler() method.
	 *
	 * \author Rolando J. Nieves
	 * \date 2012-09-24 08:12:08
	 */
	class SignalInputSource : public CoreKit::InputSource
	{

	public:
		/**
		 * \brief Create the \c signalfd() Facility to be Used by This Instance
		 *
		 * The primary constructor for the \c SignalInputSource class creates
		 * the \c signalfd() facility used to receive signal notifications via
		 * file descriptor. In addition, the constructor modifies the process
		 * signal mask so that the signal number provided as input is blocked
		 * from executing its default behavior.
		 *
		 * \param sigNum Integer identifying the signal this input source
		 *               should monitor. The integer should be as defined in
		 *               the \c csignal header file.
		 * \param intrListener \c InterruptListener derived instance that will
		 *                     be notified whenever this process receives the
		 *                     signal indicated via the \c sigNum parameter.
		 */
		SignalInputSource(int sigNum, InterruptListener *intrListener);
		/**
		 * \brief Deallocate all Resources Used by this \c SignalInputSource Instance
		 *
		 * The destructor for the \c SignalInputSource class closes down the
		 * file descriptor used to receive signal receptions. The destructor
		 * also modifies the process signal mask so that the signal this
		 * instance had previously blocked is now unblocked.
		 */
		virtual ~SignalInputSource();

		/**
		 * \brief Obtain the File Descriptor that Indicates Signal Activity
		 *
		 * The \c fileDescritptor() method is part of the \c InputSource
		 * interface, and provides the caller (more than likely a \c RunLoop
		 * instance) the file descriptor that will exhibit activity whenever
		 * the application process receives the signal watched by this
		 * \c SignalInputSource instance.
		 *
		 * \return File descriptor that will exhibit activity whenever the
		 *         application process receives the signal being watched.
		 */
		virtual int fileDescriptor() const;
		/**
		 * \brief Obtain the \c InterruptListener Instance that Will Receive Signal Reception Events
		 *
		 * The \c interruptListener() method is part of the \c InputSource
		 * interface, and provides the caller a pointer to the
		 * \c InterruptListener instance that should be notified whenever
		 * the application process receives the signal watched by this
		 * \c SignalInputSource instance.
		 *
		 * \return \c InterruptListener instance that should be called whenever
		 *         the application process receives the signal watched by
		 *         this instance.
		 */
		virtual InterruptListener* interruptListener() const;
		/**
		 * \brief Execute the \c InterruptListener::signalReceived() Callback
		 *
		 * The \c fireCallback() method is part of the \c InputSource interface
		 * and it calls the \c InterruptListener::signalReceived() method on
		 * the \c InterruptListener instance registered with this input source.
		 * \par
		 *
		 * This method is primarily used by the \c RunLoop class whenever it
		 * schedules this input source to perform work.
		 */
		virtual void fireCallback();

	private:
		/**
		 * \brief \c InterruptListener Insterested in Signal Events
		 */
		InterruptListener *m_intrListener;
		/**
		 * \brief File Descriptor for the \c signalfd() Facility
		 */
		int m_signalFd;
		/**
		 * \brief Signal Number this \c SignalInputSource Instance is Interested In
		 */
		int m_sigNum;
	};

}
#endif // !defined(EA_66751973_5C51_4baf_B560_3271FEE3738E__INCLUDED_)
