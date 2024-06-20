/**
 * \file Thread.h
 * \brief Definition of the Thread Class
 * \date 2012-09-24 08:39:58
 * \author Rolando J. Nieves
 */
#if !defined(EA_E56F5B51_5608_44a5_AE1D_5CAD8ED37A78__INCLUDED_)
#define EA_E56F5B51_5608_44a5_AE1D_5CAD8ED37A78__INCLUDED_

#include <pthread.h>
#include <semaphore.h>

#include "ThreadDelegate.h"
#include "RunLoop.h"
#include "factory.h"

namespace CoreKit
{
	class Application;

	/**
	 * \brief Concurrent Task Executed Within an \c Application
	 *
	 * The \c Thread class models concurrent tasks running within an
	 * \c Application instance as POSIX threads. These threads run within and
	 * are managed by the host \c Application instance.
	 * \par
	 *
	 * Every concurrent task created with the \c Thread instance is also
	 * afforded a \c RunLoop instance. This instance is independent from the
	 * main thread's \c RunLoop instance. It is up to the task logic to decide
	 * whether to use the \c RunLoop instance or not.
	 *
	 * \note
	 * An \c Application main \c Thread is modeled using this class, and the
	 * \c RunLoop that main \c Thread instance hosts is known to the
	 * application as the main \c RunLoop.
	 *
	 * \author Rolando J. Nieves
	 * \date 2012-08-20 13:56:31
	 */
	class Thread
	{
		RF_CK_FACTORY_COMPATIBLE(Thread);

	public:
		/**
		 * \brief Adopt an Already Running Thread
		 *
		 * The \c Thread constructor that takes in a POSIX thread ID as a
		 * parameter assumes the thread identified is already running and does
		 * not attempt to start a new one. Using this constructor, application
		 * developers are able to enhance a POSIX thread concurrent task with
		 * the \c RunLoop instance the \c Thread class affords all concurrent
		 * tasks.
		 *
		 * \note
		 * This constructor is primarily used by the \c Application class to
		 * model its main application thread.
		 *
		 * \param runningThreadId Identifier of the POSIX thread this instance
		 *                        should adopt.
		 */
		explicit Thread(pthread_t runningThreadId);

        /**
         * \brief Adopt an already running thread and use synchronization object.
		 *
		 * The \c Thread constructor that takes in a POSIX thread ID as a
		 * parameter assumes the thread identified is already running and does
		 * not attempt to start a new one. Using this constructor, application
		 * developers are able to enhance a POSIX thread concurrent task with
		 * the \c RunLoop instance the \c Thread class affords all concurrent
		 * tasks.
         *
         * This variation is meant to be used by run loops that are slaved to
         * a synchronization object.
		 *
		 * \note
		 * This constructor is primarily used by the \c Application class to
		 * model its main application thread.
		 *
		 * \param runningThreadId Identifier of the POSIX thread this instance
		 *                        should adopt.
         * \param[in] runLoopSyncObj Synchronization object that paces the host
         *            run loop.
         */
		Thread(pthread_t runningThreadId, sem_t *runLoopSyncObj);
		/**
		 * \brief Spawn a New Concurrent Task Using a POSIX Thread
		 *
		 * The primary constructor for the \c Thread class takes in enough
		 * information to start a new concurrent task managed by this
		 * \c Thread instance. The behavior of the thread will be dictated
		 * by the \c ThreadDelegate derived instance provided via parameters.
		 *
		 * \param thrDelegate \c ThreadDelegate derived instance that is
		 *                    responsible for implementing custom task
		 *                    behavior.
		 * \param hostApp \c Application instance that is hosting this thread.
		 * \param detached \c true if the thread should be created as
		 *                 "detached" (i.e., with automatic resource cleanup
		 *                 and no way of communicating status to the main
		 *                 thread upon exit; see the \c pthread_detach() man
		 *                 page for more information); \c false
		 *                 otherwise.
		 */
		Thread(ThreadDelegate* thrDelegate, Application *hostApp, bool detached = true);
		/**
		 * \brief Terminate the Concurrent Task and Release All Allocated Objects
		 *
		 * The destructor for the \c Thread class attempts to terminate the
		 * concurrent task running on this POSIX thread, then deallocates all
		 * objects created during construction and runtime by this \c Thread
		 * instance.
		 */
		virtual ~Thread();

		/**
		 * \brief Acquire the \c Application Instance Hosting this \c Thread
		 *
		 * \return \c Application instance hosting this \c Thread.
		 */
		inline Application* hostApp() const { return m_hostApp; }
		/**
		 * \brief Set the \c Application Instance Hosting this \c Thread
		 *
		 * \param theHostApp \c Application instance hosting this \c Thread.
		 */
		inline void setHostApp(Application *theHostApp) { m_hostApp = theHostApp; }
		/**
		 * \brief Acquire the \c ThreadDelegate Instance that Holds the Task Specific Behavior.
		 *
		 * \return \c ThreadDelegate instance holding the task specific behavior.
		 */
		inline ThreadDelegate* threadDelegate() const { return m_threadDelegate; };
		/**
		 * \brief Acquire the \c RunLoop Instance Created for This \c Thread Instance
		 *
		 * The \c getRunLoop() method gives \c ThreadDelegate derived instances
		 * access to the \c RunLoop instance that they are free to use in their
		 * task specific behavior logic.
		 *
		 * \return \c RunLoop instance created exclusively for this \c Thread
		 *         instance.
		 */
		virtual RunLoop* runLoop() const { return m_runLoop; };
		/**
		 * \brief Determine if This \c Thread has Already Been Cleaned Up After
		 *
		 * The \c isJoined() method is used to ascertain whether the POSIX
		 * thread managed by this \c Thread instance has already been cleaned
		 * up after (i.e., "joined") or not. \c Thread instances created as
		 * "detached" (see \c Thread::Thread(ThreadDelegate*,bool) ) always
		 * return \c false from this method.
		 *
		 * \return \c true if the thread has been "joined;" \c false otherwise
		 */
		virtual bool isJoined() const { return (m_threadState == Thread::JOINED); };
		/**
		 * \brief Obtain the POSIX Thread ID Associated with This \c Thread Instance
		 *
		 * The \c getThreadId() is used by classes that need to know the POSIX
		 * thread ID of the concurrent task hosted by this \c Thread instance.
		 *
		 * \return POSIX Thread ID of the thread used to host the concurrent
		 *         task behavior.
		 */
		virtual pthread_t threadId() const { return m_threadId; };

		/**
		 * \brief Clean Up After ("join") a POSIX Thread
		 *
		 * The \c join() method is used to reap the resources from a defunct
		 * POSIX thread. This method call is not necessary if the \c Thread
		 * instance was created as "detached" (see
		 * Thread::Thread(ThreadDelegate*,bool) ).
		 */
		virtual void join();

	private:
		/**
		 * \brief Constants Describing the Current State of a Thread
		 */
		enum ThreadState
		{
			RUNNING,
			STOPPED,
			JOINED
		} m_threadState;
		/**
		 * \brief \c RunLoop Instance Created Exclusively for Use by This \c Thread Task Behavior
		 */
		RunLoop* m_runLoop;
		/**
		 * \brief \c ThreadDelegate Instance that Holds Task Specific Behavior
		 */
		ThreadDelegate* m_threadDelegate;
		/**
		 * \brief POSIX Thread ID of The Concurrent Task
		 */
		pthread_t m_threadId;
		/**
		 * \brief Flag Indicating Whether the Thread Was Created as Detached or Not
		 */
		bool m_isDetached;
		/**
		 * \brief Application Instance Managing this Concurrent Thread
		 */
		Application *m_hostApp;

        static void* threadKickoffRoutine(void *userData);
	};

}
#endif // !defined(EA_E56F5B51_5608_44a5_AE1D_5CAD8ED37A78__INCLUDED_)
