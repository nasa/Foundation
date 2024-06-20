/**
 * \file AppDelegate.h
 * \brief Definition of the AppDelegate Class
 * \date 2012-09-24 08:39:56
 * \author Rolando J. Nieves
 */
#if !defined(EA_D7495A3D_45B3_4f4c_9DF8_A190837A18E2__INCLUDED_)
#define EA_D7495A3D_45B3_4f4c_9DF8_A190837A18E2__INCLUDED_

namespace CoreKit
{
	class Application;

	/**
	 * \brief Delegate for the CoreKit::Application Class.
	 *
	 * The \c CoreKit::AppDelegate class provides framework application
	 * developers with the ability to extend the behavior of the
	 * \c CoreKit::Application class via delegation. At certain times during
	 * the lifecycle of a framework application, \c CoreKit::Application will
	 * call on its delegate to perform any required tasks to implement the
	 * application's unique behavior. Classes deriving from
	 * \c CoreKit::AppDelegate are not obligated to provide
	 * application-specific implementations to any of these methods. Not
	 * implementing any of the delegate methods, however, results in an
	 * application with very little unique behavior.
	 *
	 * \author Rolando J. Nieves
	 * \date 20-Aug-2012 1:55:55 PM
	 */
	class AppDelegate
	{

	public:
		/**
		 * \brief Primary Constructor for the \c AppDelegate interface.
		 *
		 * The primary construtor for \c CoreKit::AppDelegate initializes all
		 * of the instance fields to nominal values. Classes implementing the
		 * \c CoreKit::AppDelegate interface need not call this constructor
		 * explicitly.
		 */
		AppDelegate();
		/**
		 * \brief Destructor for the \c AppDelegate interface.
		 *
		 * The destructor for \c CoreKit::AppDelegate contains no
		 * behavior specific to this class. The constructor is made \c virtual
		 * in order to accommodate inheritance hierarchies.
		 */
		virtual ~AppDelegate();

		/**
		 * \brief Access the \c hostApp Field
		 *
		 * The \c hostApp() accessor method provides read-only access to the
		 * \c hostApp field. The field contains a pointer to the
		 * \c CoreKit::Application instance hosting this
		 * \c CoreKit::AppDelegate derived instance. This field is not set
		 * automatically for the application developer. Instead, the developer
		 * must set it using the \c setHostApp() method during one of the
		 * lifecycle delegation calls.
		 *
		 * \return CoreKit::Application* Pointer to the \c Application instance
		 *         hosting this \c AppDelegate instance
		 */
		inline CoreKit::Application* hostApp() const { return m_hostApp; }
		/**
		 * \brief Modify the contents of the \c hostApp field.
		 *
		 * The \c setHostApp() modifier method lets application developers
		 * store a pointer to the \c Application instance that is hosting their
		 * \c AppDelegate derived instance. It is recommended that this method
		 * be used to initialize the value of the field at the earliest
		 * possible lifecycle delegate method implemented by the framework
		 * developer.
		 *
		 * \param hostApp Pointer to the \c Application instance hosting this
		 *                \c AppDelegate derived instance.
		 */
		inline void setHostApp(CoreKit::Application *hostApp) { m_hostApp = hostApp; }

		/**
		 * \brief Delegated behavior immediately after initialization.
		 *
		 * The \c applicationDidInitialize() method is called by the
		 * \c Application instance hosting this \c AppDelegate derived
		 * instance immediately after \c Application performs all of the
		 * required framework initialization. This is the method that the
		 * \c Application class uses to delegate the execution of
		 * application-specific behavior to a framework application. By the
		 * time \c Application makes this delegation call, the following would
		 * have already been done:
		 *  - The application's main \c Thread object would have been created,
		 *    along with its supporting \c RunLoop object.
		 *  - The application's logger object (\c AppLog) would have been
		 *    created and initialized.
		 *  - The termination handlers (SIGTERM and SIGINT) would have already
		 *    been configured.
		 *  - The command line, as configured and provided to the host
		 *    \c Application instance, would have been processed; all the
		 *    command-line arguments that were of interest to the application
		 *    would be ready for query.
		 *
		 *  \param theApplication Pointer to the \c Application instance
		 *                        hosting this \c AppDelegate derived instance.
		 */
		virtual void applicationDidInitialize(Application* theApplication);
		/**
		 * \brief Delegated behavior immediately after application main loop exit.
		 *
		 * The \c applicationDidTerminate() method is called by the
		 * \c Application instance hosting this \c AppDelegate derived instance
		 * immediately after the application's main loop exits and the
		 * application is ready to terminate. By the time \c Application makes
		 * this delegation call, the following would have already been done:
		 *  - The main \c Thread \c RunLoop would have terminated.
		 *  - All secondary application \c Thread objects, along with their
		 *    \c RunLoop would have terminated.
		 *
		 * \param theApplication Pointer to the \c Application instance hosting
		 *                       this \c AppDelegate derived instance.
		 */
		virtual void applicationDidTerminate(Application* theApplication);
		/**
		 * \brief Delegated behavior just before main loop start.
		 *
		 * The \c applicationStarting() method is called by the \c Application
		 * instance hosting this \c AppDelegate derived instance just prior to
		 * the \c Application surrendering control to the main \c Thread
		 * \c RunLoop.
		 *
		 * \param theApplication Pointer to the \c Application instance hosting
		 *                       this \c AppDelegate derived instance.
		 */
		virtual void applicationStarting(Application* theApplication);
		/**
		 * \brief Delegated behavior just prior to falling out of the main loop
		 *
		 * The \c applicationWillTerminate() method is called by the
		 * \c Application instance hosting this \c AppDelegate derived instance
		 * just as a request to terminate the application (either via system
		 * signal or \c Application::requestTermination() ) was processed and
		 * the application is ready to terminate. By the time \c Application
		 * makes this call, the last iteration of the application's main loop
		 * would have already executed.
		 *
		 * \param theApplication Pointer to the \c Application instance hosting
		 *                       this \c AppDelegate derived instance.
		 */
		virtual void applicationWillTerminate(Application* theApplication);

	private:
		/**
		 * \brief \c Application instance hosting this \c AppDelegate derived instance.
		 *
		 * This field is not set automatically by this class. Instead, it is the
		 * responsibility of the application developer to set it via
		 * \c setHostApp()
		 */
		CoreKit::Application *m_hostApp;
	};

}
#endif // !defined(EA_D7495A3D_45B3_4f4c_9DF8_A190837A18E2__INCLUDED_)
