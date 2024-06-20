/**
 * \file Application.h
 * \brief Definition of the Application Class
 * \date 2012-09-24 08:39:56
 * \author Rolando J. Nieves
 */
#if !defined(EA_9A8D031A_97B7_4f9d_A36D_E3106FDFEF9E__INCLUDED_)
#define EA_9A8D031A_97B7_4f9d_A36D_E3106FDFEF9E__INCLUDED_

#include <semaphore.h>
#include <csignal>
#include <string>
#include <vector>
#include <map>
#include <sstream>

#include <CoreKit/AppDelegate.h>
#include <CoreKit/Thread.h>
#include <CoreKit/ThreadDelegate.h>
#include <CoreKit/InterruptListener.h>
#include <CoreKit/AppLog.h>

namespace CoreKit
{
    /**
     * \brief Controls all Operations Related to a Framework Application.
     *
     * The \c CoreKit::Application class is the principal designed to control
     * everything that goes on during the execution of a framework application.
     * Instances of this class control an application's main thread of
     * execution, along with any subordinate threads created by the
     * application-specific behavior. In addition, instances of this class
     * control the initialization and termination of a framework application.
     * \par
     *
     * Framework applications integrate their unique behavior with this class
     * using a delegate class that implements the \c CoreKit::AppDelegate
     * interface. The delegate class should add application-specific behavior
     * using the application lifecycle events exposed by the
     * \c CoreKit::AppDelegate interface.
     *
     * \note
     * Although it is possible to customize the behavior of this class via
     * inheritance, the recommended method for implementing application
     * specific behavior is via delegation as explained above.
     *
     * \author Rolando J. Nieves
     * \date 17-Aug-2012 6:22:45 PM
     */
    class Application : public CoreKit::InterruptListener
    {
    public:

        /**
         * \brief Models a Command-Line Flag Definition
         *
         * The \c Application::CmdLineArg class is used to declare and define
         * a command line flag that the \c Application class should take
         * into account when parsing the list of command line arguments passed
         * to it during application initialization.
         *
         * \author Rolando J. Nieves
         * \date 17-Aug-2012 6:22:45 PM
         */
        struct CmdLineArg
        {
            RF_CK_FACTORY_COMPATIBLE(CmdLineArg);

            /**
             * \brief Command Line Flag (without the preceding --)
             *
             * The \c longOption attribute contains the command line flag
             * associated with this argument definition.
             */
            const std::string longOption;
            /**
             * \brief Determines Whether the Command Line Flag Requires an Argument
             *
             * The \c requiresArg attribute lets the \c Application class know
             * if the command line flag defined in this \c CmdLineArg instance
             * requires an argument.
             * \par
             *
             * Should the command line flag require an argument, and the list
             * of command line arguments provided during initialization does
             * not conform to this definition, \c Application will throw an
             * exception.
             */
            const bool requiresArg;
            /**
             * \brief Help String Message Associated with the Command Line Flag
             *
             * The \c optionHelpStr attribute contains a short, plain english
             * explanation for the command line flag modeled by this
             * \c CmdLineArg instance. This string is used when the
             * \c Application class generates a help screen documenting all
             * command line flags.
             */
            const std::string optionHelpStr;

            /**
             * \brief Initialize All the Fields in the \c CmdLineArg Instance
             *
             * The primary constructor for the \c CmdLineArg class initializes
             * the value of all the fields in an instance.
             *
             * \note
             * This is the only way to set the field values since after
             * construction the fields are invariant.
             *
             * \param theLongOption The command line flag's text (as expected
             *                      from the command line, without the --
             *                      prefix)
             * \param theArgReq \c true if an argument is required with this
             *                  command line flag; \c false otherwise.
             * \param theOptionHelpStr Brief line of text that describes the
             *                         meaning and usage of the command line
             *                         flag.
             */
            inline CmdLineArg(
                std::string const& theLongOption,
                bool theArgReq,
                std::string const& theOptionHelpStr
            ):
                longOption(theLongOption),
                requiresArg(theArgReq),
                optionHelpStr(theOptionHelpStr)
            {}

            /**
             * \brief Copy the Contents of Another \c CmdLineArg Instance
             *
             * The copy constructor for the \c CmdLineArg class makes a deep
             * copy between two instances, copying the field values
             * accordingly. This constructor is primarily meant for use in STL
             * collections (such as \c std::vector ).
             *
             * \param other Reference to the other \c CmdLineArg instance whose
             *              values should be copied onto this instance.
             */
            inline CmdLineArg(CmdLineArg const& other):
                longOption(other.longOption),
                requiresArg(other.requiresArg),
                optionHelpStr(other.optionHelpStr)
            {}

            /**
             * \brief Create a Help String Suitable for Printing to the Screen
             *
             * The \c formatHelpStr() method takes the command line flag's
             * definition from the fields in this instance and produces a
             * human readable text string that may be used as part of a help
             * screen for the application.
             *
             * \return Help string formatted with the command line flag left
             *         justified and the developer-provided help string
             *         indented.
             */
            std::string formatHelpStr() const;
        };

        /**
         * \brief Initialize the \c Application Instance with an \c AppDelegate
         *
         * The primary constructor for the \c Application instance initializes
         * all of the instance fields to nominal values, and creates the
         * relationship between an \c Application instance and an
         * \c AppDelegate instance.
         *
         * \param theAppDelegate Pointer to the \c AppDelegate that this
         *                       \c Application instance should host.
         */
        explicit Application(AppDelegate* theAppDelegate);

        /**
         * \brief Free Up All Resources Used by the \c Application Instance
         *
         * The destructor for the \c Application class terminates all
         * subordinate threads started by application specific code, as well
         * as deallocate all objects instantiated by this \c Application
         * instance.
         */
        virtual ~Application();

        /**
         * \brief Handle \c SIGTERM and \c SIGINT Signals Delivered to the \c Application
         *
         * The \c signalReceived() method is programmed to be called by the
         * main \c Thread \c RunLoop whenever the application process
         * receives a \c SIGTERM (Terminate) or \c SIGINT (Interrupt) signal.
         *
         * \param signalNumber Number of the signal received by the process.
         */
        virtual void signalReceived(int signalNumber);

        /**
         * \brief Initialize the \c Application Instance for Execution
         *
         * The \c initialize() method creates and initializes the main
         * \c Thread and \c RunLoop. The method also processes all the
         * command line arguments passed into the application process.
         * \par
         *
         * Towards the end of this method's logic, the
         * \c AppDelegate::applicationDidInitialize() method is called on the
         * delegate submitted during construction time.
         *
         * \param appName Name the application should use in all log messages.
         * \param argCount Number of command line arguments provided to the
         *                 application process.
         * \param argVals Character string array holding all command line
         *                arguments provided to the application process.
         */
        void initialize(
            std::string appName,
            int argCount,
            char const** argVals
        );

        /**
         * \brief Send a \c SIGTERM Signal to the application process.
         *
         * The \c requestTermination() method is used to have the application
         * process initiate termination procedures by sending the \c SIGTERM
         * signal to the process. Receipt of the signal will then trigger
         * termination procedures in the \c Application main \c Thread
         * as well as subordinate \c Thread objects.
         */
        void requestTermination();

        /**
         * \brief Start a Subordinate \c Thread on this \c Application
         *
         * The \c spawnThread() method is used to start a subordinate
         * \c Thread running alongside the main \c Thread. The new
         * subordinate \c Thread begins execution soon after this call is
         * finished. The thread may be created \e joinable or \e detached.
         *
         * \param thrDelegate Pointer to the \ref ThreadDelegate that contains
         *                    the \ref Thread specific behavior.
         * \param detached \c true if the thread should be created as
         *                 \e detached; \c false otherwise.
         */
        virtual void spawnThread(
            ThreadDelegate* thrDelegate,
            bool detached = true
        );

        /**
         * \brief Start Execution of the Main \c RunLoop
         *
         * The \c start() method surrenders execution control to the main
         * \c Thread \c RunLoop .
         * \par
         *
         * Prior to surrendering to the main \c RunLoop, this method calls
         * the \c AppDelegate::applicationStarting() method in the delegate
         * provided at construction time.
         */
        void start();

        /**
         * \brief Add a Command Line Flag Definition to the Command Line Parser
         *
         * The \c addCmdLineArgDef() method accepts command line flag
         * definitions from the framework application. These definitions are
         * used by the \c Application class during command line parsing.
         *
         * \note
         * All of the command line flag definitons must be submitted to the
         * \c Application instance prior to calling the \c initialize() method.
         *
         * \param theNewArg \c CmdLineArg instance that defines the new command
         *                  line flag.
         */
        void addCmdLineArgDef(CmdLineArg const& theNewArg);

        /**
         * \brief Retrieve the Argument Associated with a Command Line Flag
         *
         * The \c getCmdLineArgFor() method is used to retrieve the command
         * line argument that was identified as the argument to a
         * pre-configured command line flag.
         *
         * \param longOption Command line flag whose argument is sought.
         * \return For command line flags that required an argument, this
         *         method returns the argument as a string object if the
         *         command line flag was present in the list of command line
         *         arguments passed to the application process. For command
         *         line flags that do not require an argument, this method
         *         returns a non-empty string object if the command line flag
         *         was passed to the process, and an empty string if the
         *         command line flag was not passed to the process.
         */
        std::string const& getCmdLineArgFor(
            std::string const& longOption
        ) const;

        /**
         * \brief Access the Main \c Thread \c RunLoop Object.
         *
         * The \c getMainRunLoop() method is used to acquire a pointer to the
         * \c RunLoop object contained in the application's main thread.
         *
         * \return A pointer to the main \c Thread \c RunLoop object.
         */
        RunLoop* getMainRunLoop() const;

        /**
         * \brief Access the Calling \c Thread \c RunLoop Object.
         *
         * The \c getCurrentRunLoop() method is used to acquire a pointer to
         * the \c RunLoop object associated in the thread of execution used to
         * call this method. Calling this method from the main thread yields
         * the same result as the \c getMainRunLoop() method. Calling this
         * from a subordinate thread yields the \c RunLoop associated with the
         * \c Thread object representing the calling thread, if any (i.e.,
         * calling this method from a thread not created via the spawnThread()
         * method yields \c NULL as a result).
         *
         * \return Pointer to the \c RunLoop instance associated with the
         *         calling thread; \c NULL if no \c RunLoop is associated with
         *         the calling thread.
         */
        virtual RunLoop* getCurrentRunLoop() const;

        /**
         * \brief Access the \c AppLog Object Associated with this \c Application
         *
         * The \c log() method is meant to give application specific code
         * access to the central logging facility created and maintained by the
         * \c Application class.
         *
         * \note
         * The application log object is available for use immediately after
         * \c Application instance construction. However, prior to application
         * initialization, this logging facility is configured to output all
         * messages submitted to it regardless of log level, and it will output
         * messages with an application name of "<unknown>"
         *
         * \return \c AppLog object that has been properly configured by the
         *         \c Application instance.
         */
        AppLog& log();

    private:
        /**
         * \brief Parse Through All Application Command Line Arguments
         *
         * The \c processCmdLine() method is an internal utility function that
         * contains all of the logic used to parsed through the command line
         * arguments fed in to the \c Application process. This method, after
         * parsing through the arguments, populates a map with entries for each
         * command line flag provided as input. \par
         *
         * This method is also responsible for producing an application help
         * screen if the application process is called with a \c --help command
         * line flag.
         *
         * \param argCount Number of command line arguments passed to the
         *                 application process.
         * \param argVals Character string array containing all arguments
         *                passed to the application process.
         */
        void processCmdLine(int argCount, char const** argVals);

        static const std::string SCHED_SYNC_FLAG;
        static const std::string GDB_FLAG;
        static const std::string DAEMON_FLAG;
        static const std::string PID_BASE_NAME_FLAG;

        typedef std::map< std::string, std::string > ArgValMap;
        /**
         * \brief \c AppDelegate Instance Hosted by this \c Application
         */
        AppDelegate* m_appDelegate;
        /**
         * \brief Collection of \c Thread Objects Associated with this \c Application
         */
        std::vector<Thread*> m_appThreads;
        /**
         * \brief Command Line Flag Definitions to Use When Parsing Command Line Arguments
         */
        std::vector<CmdLineArg*> m_argDefs;
        /**
         * \brief Main \c Thread for this \c Application Instance
         */
        Thread* m_mainThread;
        /**
         * \brief Map of Command Line Flag Arguments as Parsed
         */
        ArgValMap m_argValMap;
        /**
         * \brief \c Application Wide Log Facility Object
         */
        AppLog *m_log;
        /**
         * \brief System Signal Set State Prior to \c Application Initialization
         */
        sigset_t m_origSigSet;
        /**
         * \brief Synchronization Object Used to Tie Into an External Scheduler.
         */
        sem_t *m_schedSyncObj;
        /**
         * \brief Flag Used to Bypass Application Startup.
         * This flag is used exclusively by the command-line parser to indicate
         * the user invoked the application just to obtain help information.
         */
        bool m_inhibitStartup;
    };

    /**
     * \brief Global variable for storing single application instance.
     */
    extern Application *G_MyApp;
} // end namespace CoreKit

#endif // !defined(EA_9A8D031A_97B7_4f9d_A36D_E3106FDFEF9E__INCLUDED_)

// vim: set ts=4 sw=4 expandtab:
