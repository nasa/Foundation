/**
 * \file Application.cpp
 * \brief Implementation of the Application Class
 * \date 2012-09-24 08:39:56
 * \author Rolando J. Nieves
 */

#if defined(HAVE_SYSLOG_H) && (HAVE_SYSLOG_H == 1)
# include <syslog.h>
#endif /* defined(HAVE_SYSLOG_H) && (HAVE_SYSLOG_H == 1) */
#include <unistd.h>
#include <sys/stat.h>
#include <getopt.h>
#include <fcntl.h>
#include <cstdlib>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <algorithm>
#include <functional>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

#include <CoreKit/PreconditionNotMetException.h>
#include <CoreKit/RuntimeErrorException.h>
#include <CoreKit/InvalidInputException.h>
#include <CoreKit/OsErrorException.h>

#include "Application.h"


using std::for_each;
using std::ptr_fun;
using std::vector;
using std::find_if;
using std::bind2nd;
using std::string;
using std::stringstream;
using std::ends;
using std::cerr;
using std::endl;
using std::cout;
using std::max;
using std::ofstream;
using CoreKit::Application;
using CoreKit::ThreadDelegate;
using CoreKit::Thread;
using CoreKit::RunLoop;
using CoreKit::PreconditionNotMetException;
using CoreKit::RuntimeErrorException;
using CoreKit::InvalidInputException;
using CoreKit::AppLog;
using CoreKit::OsErrorException;
using CoreKit::construct;
using CoreKit::destroy;

static const string EMPTY_STRING("");
const string Application::SCHED_SYNC_FLAG("sched-sync");
const string Application::GDB_FLAG("gdb");
const string Application::DAEMON_FLAG("daemon");
const string Application::PID_BASE_NAME_FLAG("pid-base-name");

namespace CoreKit
{

Application *G_MyApp = nullptr;

} // end namespace CoreKit

/**
 * \brief Join and Delete an \c Application Subordinate Thread
 *
 * The \c joinAndDeleteAppThread() local function is used as part of a
 * \c std::for_each loop in the \c Application class that is meant to clean
 * up all subordinate threads during termination.
 *
 * \param theThread \c Thread object that is to be joined then deleted.
 */
static void joinAndDeleteAppThread(Thread *theThread)
{
    if (!theThread->isJoined())
    {
        theThread->join();
    }

    destroy(theThread);
}

/**
 * \brief Test Whether a POSIX Thread ID Matches That of a \c Thread Instance
 *
 * The \c matchPthreadId() local function is used as part of a \c std::find_if
 * algorithm in order to locate the \c Thread object that represents the POSIX
 * thread with the identifier provided.
 *
 * \param aThread \c Thread object to use in the comparison.
 * \param threadId \c POSIX thread ID to use in the comparison.
 *
 * \return \c true if the \c Thread object represents the POSIX thread
 *         identified by the provided thread ID; \c false otherwise.
 */
static bool matchPthreadId(Thread *aThread, pthread_t threadId)
{
    return (aThread->threadId() == threadId);
}

/**
 * \brief Request Termination on a \c Thread \c RunLoop
 *
 * The \c requestRunLoopTerm() local function is used as part of a
 * \c std::for_each loop that requests the \c RunLoop termination of a given
 * \c Thread object. It is primarily used during shutdown procedures.
 *
 * \param aThread \c Thread instance whose \c RunLoop should be requested to
 *                terminate.
 */
static void requestRunLoopTerm(Thread *aThread)
{
    aThread->runLoop()->terminate();
}

/**
 * \brief Print the Help String Associated with a Command Line Flag
 *
 * The \c printCmdLineHelpStr() local function is used as part of a
 * \c std::for_each loop that prints out a help screen for the \c Application
 * process.
 *
 * \param anArg \c CmdLineArg instance whose help string will be printed.
 */
static void printCmdLineHelpStr(Application::CmdLineArg *anArg)
{
    cout << anArg->formatHelpStr() << endl;
}

string Application::CmdLineArg::formatHelpStr() const
{
    stringstream theFormattedStr;
    char padding[] = { "                 " };
    int nullPos = 0;
    int paddingReq = 0;

    /*
     * Figure out how much space is required between the command line flag and
     * its help string. The difference in length between the maximum padding
     * and the command line flag points to where the padding string should be
     * forcibly terminated (with a NULL character).
     */
    paddingReq = strlen(padding) - longOption.length();
    nullPos = max(0,paddingReq);
    padding[nullPos] = 0x00;

    theFormattedStr << "--" << longOption << padding << optionHelpStr << ends;

    return theFormattedStr.str();
}


Application::Application(AppDelegate* theAppDelegate):
    m_appDelegate(theAppDelegate),
    m_log(nullptr),
    m_mainThread(nullptr),
    m_schedSyncObj(nullptr),
    m_inhibitStartup(false)
{
    /*
     * Create a log service object that will function during the application's
     * pre-initialization logic.
     */
    m_log = construct(AppLog::myType(), "<uninitialized>");
    this->addCmdLineArgDef(
        CmdLineArg(
            "help",
            false,
            "Display this help message."
        )
    );
    this->addCmdLineArgDef(
        CmdLineArg(
            Application::SCHED_SYNC_FLAG,
            true,
            "Scheduler synchronization object to use."
        )
    );
    this->addCmdLineArgDef(
        CmdLineArg(
            "log-level",
            true,
            "Log message at or above=(DEBUG|INFO|WARN|ERR)"
        )
    );
    this->addCmdLineArgDef(
        CmdLineArg(
            Application::GDB_FLAG,
            false,
            "Do not trap SIGINT (only useful when running from GDB)."
        )
    );
    this->addCmdLineArgDef(
        CmdLineArg(
            Application::DAEMON_FLAG,
            false,
            "Start the application in the background."
        )
    );
    this->addCmdLineArgDef(
        CmdLineArg(
            Application::PID_BASE_NAME_FLAG,
            true,
            "Use the specified string as the PID file base name instead of "
            "the application name."
        )
    );

    /*
     * If a delegate was submitted for this Application instance to host (it is
     * valid to not provide one, BTW ... somewhat unorthodox but still valid)
     * let the delegate know who's its host.
     */
    if (theAppDelegate != nullptr)
    {
        theAppDelegate->setHostApp(this);
    }
}


Application::~Application()
{
    /*
     * Deallocate all of the command line flag definition objects.
     */
    for_each(
        m_argDefs.begin(),
        m_argDefs.end(),
        ptr_fun(&destroy<Application::CmdLineArg>)
    );
    m_argDefs.clear();

    /*
     * Join (if applicable) and delete all subordinate threads.
     */
    for_each(
        m_appThreads.begin(),
        m_appThreads.end(),
        ptr_fun(&joinAndDeleteAppThread)
    );
    m_appThreads.clear();

    /*
     * Deallocate the main thread object
     */
    destroy(m_mainThread);
    m_mainThread = nullptr;

    /*
     * Close the scheduler synchronization object, if one was used.
     */
    if (m_schedSyncObj != nullptr)
    {
        sem_close(m_schedSyncObj);
        m_schedSyncObj = nullptr;
    }

    destroy(m_log);
    m_log = nullptr;
}


void
Application::signalReceived(int signalNumber)
{
    /*
     * Give the application delegate (if one was configured) the chance to
     * perform some pre-termination logic.
     */
    if (m_appDelegate != nullptr)
    {
        m_appDelegate->applicationWillTerminate(this);
    }

    /*
     * Terminate the run loop of all applications threads (both subordinate and
     * the main thread).
     */
    for_each(
        m_appThreads.begin(),
        m_appThreads.end(),
        ptr_fun(&requestRunLoopTerm)
    );
    m_mainThread->runLoop()->terminate();
}


void
Application::initialize(std::string appName, int argCount, char const** argVals)
{
    pid_t daemonPid;
    pid_t sessionId;
    stringstream pidFileName;
    FILE *pidFile = nullptr;
    std::string schedSyncObj;
    std::string pidBaseName = appName;
    bool daemonMode = false;
    string logLevel = "DEBUG";

    if (m_mainThread != nullptr)
    {
        throw PreconditionNotMetException("Application not previously initialized.");
    }

    /*
     * Go through all of the command line arguments provided to the application
     * process, storing all information pertaining to the pre-configured
     * command line flags.
     */
    this->processCmdLine(argCount, argVals);

    /*
     * If the --help command line flag was given, print out an application help
     * page, then exit.
     */
    if (!m_argValMap["help"].empty())
    {
        cout << "Command line options:" << endl;
        for_each(m_argDefs.begin(), m_argDefs.end(), &printCmdLineHelpStr);
        m_inhibitStartup = true;
        return;
    }

    /*
     * If the daemon flag was given, fork a new process, write the PID of the
     * fork'd process to a PID file then close all standard input/output
     * descriptors.
     */
    daemonMode = !this->getCmdLineArgFor(Application::DAEMON_FLAG).empty();
    if (daemonMode)
    {
        if (!this->getCmdLineArgFor(Application::PID_BASE_NAME_FLAG).empty())
        {
            pidBaseName = this->getCmdLineArgFor(Application::PID_BASE_NAME_FLAG);
        }
        daemonPid = fork();
        if (daemonPid < 0)
        {
            throw OsErrorException("fork()", errno);
        }
        if (daemonPid > 0)
        {
            //
            // Parent process. Write PID and exit
            //
            if (getuid() == 0)
            {
                pidFileName << "/var/run/" << pidBaseName << ".pid" << ends;
            }
            else
            {
                pidFileName << "./" << pidBaseName << ".pid" << ends;
            }
            pidFile = fopen(pidFileName.str().c_str(), "w");
            if (pidFile != nullptr)
            {
                fprintf(pidFile, "%d\n", daemonPid);
                fclose(pidFile);
                pidFile = nullptr;
            }
            else
            {
                throw OsErrorException("Unable to write PID file", errno);
            }
            m_inhibitStartup = true;
            return;
        }
        else
        {
            //
            // Child process. Do all the daemon stuff.
            //
            sessionId = setsid();
            if (sessionId < 0)
            {
                throw OsErrorException("setsid()", errno);
            }

            if ((getuid() == 0) && (chdir("/") < 0))
            {
                throw OsErrorException("chdir()", errno);
            }

            close(STDIN_FILENO);
            close(STDOUT_FILENO);
            close(STDERR_FILENO);
            
            open("/dev/null", O_RDONLY);
            open("/dev/null", O_WRONLY);
            open("/dev/null", O_WRONLY);
        }
    }

    /*
     * Get rid of the pre-initialization log service object that was created at
     * construction time.
     */
    if (m_log != nullptr)
    {
        destroy(m_log);
    }

    /*
     * Create a new log service object with the proper application name.
     */
    m_log = construct(AppLog::myType(), appName, !daemonMode);

    /*
     * Configure the application log service object so that log messages below
     * the minimum level specified at the command line do not get generated.
     */
    if (!(logLevel = m_argValMap["log-level"]).empty())
    {
        if ("DEBUG" == logLevel)
        {
            m_log->setMinLevel(AppLog::LL_DEBUG);
        }
        else if ("INFO" == logLevel)
        {
            m_log->setMinLevel(AppLog::LL_INFO);
        }
        else if ("WARN" == logLevel)
        {
            m_log->setMinLevel(AppLog::LL_WARNING);
        }
        else if ("ERR" == logLevel)
        {
            m_log->setMinLevel(AppLog::LL_ERROR);
        }
        else
        {
            cerr << "WARNING: Unknown log level \"" << logLevel << "\"." << endl;
        }
    }

    /*
     * Create the Thread object that represents the main thread. The use of
     * this constructor instructs the Thread class to not spawn off a new
     * thread, but instead "adopt" the thread identified by the POSIX thread
     * ID provided as input (which happens to be the POSIX thread ID of the
     * main thread).
     */
    schedSyncObj = this->getCmdLineArgFor(Application::SCHED_SYNC_FLAG);
    if (!schedSyncObj.empty())
    {
        /*
         * If the --sched-sync argument was provided, we will create the main
         * run loop so that it synchronizes with an externally maintained
         * synchronization object.
         */
        m_schedSyncObj = sem_open(schedSyncObj.c_str(), O_CREAT, S_IRUSR | S_IWUSR, 0);
        if (SEM_FAILED == m_schedSyncObj)
        {
            throw OsErrorException("sem_open", errno);
        }
        m_mainThread = construct(Thread::myType(), pthread_self(), m_schedSyncObj);
    }
    else
    {
        m_mainThread = construct(Thread::myType(), pthread_self());
    }

    /*
     * Set up the termination signal handling for the application, by
     * requesting notifications on the SIGTERM and SIGINT signals.
     */
    pthread_sigmask(SIG_SETMASK, nullptr, &m_origSigSet);
    if (this->getCmdLineArgFor(Application::GDB_FLAG).empty())
    {
        m_mainThread->runLoop()->registerSignalHandler(SIGINT, this);
    }
    m_mainThread->runLoop()->registerSignalHandler(SIGTERM, this);

    /*
     * Give the application delegate (if one was configured), the chance to
     * execute some post-initialization logic.
     */
    if (m_appDelegate != nullptr)
    {
        m_appDelegate->applicationDidInitialize(this);
    }
}


void
Application::requestTermination()
{
    raise(SIGTERM);
}


void
Application::spawnThread(ThreadDelegate* thrDelegate, bool detached)
{
    Thread *newThread = nullptr;

    if (nullptr == m_mainThread)
    {
        throw PreconditionNotMetException("Application already initialized.");
    }

    /*
     * Create a new subordinate Thread instance that hosts the ThreadDelegate
     * instance provided as input. Remember this new Thread object in our
     * Application instance for future clean-up.
     */
    newThread = construct(Thread::myType(), thrDelegate, this, detached);
    m_appThreads.push_back(newThread);
}


void
Application::start()
{
    /*
     * When the user specifies the "--help" command-line flag, this boolean
     * flag will be true, and the application will quit right away without
     * initializing the delegate.
     */
    if (m_inhibitStartup)
    {
        return;
    }

    if (nullptr == m_mainThread)
    {
        throw PreconditionNotMetException("Application already initialized.");
    }

    /*
     * Give the application delegate the opportunity to execute some pre-start
     * logic prior to surrendering control to the main run loop.
     */
    if (m_appDelegate != nullptr)
    {
        m_appDelegate->applicationStarting(this);
    }

    m_mainThread->runLoop()->run();

    /*
     * Give the application delegate the opportunity to execute some post-stop
     * logic prior to exiting the start() method (and, for all intents and
     * purposes, exiting the application).
     */
    if (m_appDelegate != nullptr)
    {
        m_appDelegate->applicationDidTerminate(this);
    }

    /*
     * Restore the processes' signal mask to what it was prior to application
     * initialization.
     */
    pthread_sigmask(SIG_SETMASK, &m_origSigSet, nullptr);
}

void
Application::addCmdLineArgDef(Application::CmdLineArg const& theNewArg)
{
    m_argDefs.push_back(
        construct(Application::CmdLineArg::myType(), theNewArg)
    );
}

string const&
Application::getCmdLineArgFor(string const& longOption) const
{
    std::map<std::string,std::string>::const_iterator theVal = m_argValMap.end();

    /*
     * Find the command line argument value associated with the command line
     * flag provided as input.
     */
    theVal = m_argValMap.find(longOption);
    if (theVal == m_argValMap.end())
    {
        /*
         * If a value for the flag was not found, return the empty string.
         */
        return EMPTY_STRING;
    }
    else
    {
        /*
         * Each entry in a map is a "pair" or "tuple", consisting of a key and
         * a value. The key is located in the first slot of the tuple; the
         * value is located in the second slot.
         */
        return (*theVal).second;
    }
}

void
Application::processCmdLine(int argCount, char const** argVals)
{
    struct option *theOpts = nullptr;
    std::vector<CmdLineArg*>::iterator cmdLineIter = m_argDefs.end();
    int idx = 0;
    int getOptRet = 0;
    int optIndex = 0;
    stringstream errorMsg;

    /*
     * Create the array of data structures that the operating system
     * "getopt_long()" function may use to parse through the command line
     * arguments.
     */
    theOpts = new option[m_argDefs.size() + 1];
    memset(theOpts, 0x00, (m_argDefs.size() + 1) * sizeof(option));
    for (cmdLineIter = m_argDefs.begin(); cmdLineIter != m_argDefs.end(); ++cmdLineIter)
    {
        theOpts[idx].name = (*cmdLineIter)->longOption.c_str();
        theOpts[idx].has_arg = static_cast<int>((*cmdLineIter)->requiresArg);
        theOpts[idx].val = 0;
        idx++;
    }

    /*
     * Use "getopt_long()" to iterate over all of the command line arguments.
     * Every time the function is called, it modifies a global "optarg"
     * variable (defined in the C library) while returning the index of the
     * command line flag that matches. If no match is found or a problem
     * occurs, "getopt_long()" returns a negative number. Refer to the
     * "getopt_long()" manual page for further details.
     */
    while((getOptRet = getopt_long(argCount, const_cast<char* const*>(argVals),
            "", &theOpts[0], &optIndex)) != -1)
    {
        /*
         * If a command line flag was missing an argument, report and bail.
         */
        if ('?' == getOptRet)
        {
            stringstream invalInpMsg;
            invalInpMsg << "Command line flag " << theOpts[optIndex].name << " argument." << ends;
            throw InvalidInputException(invalInpMsg.str(), "<missing>");
        }

        /*
         * If "getopt_long()" reported an unknown error, bail.
         */
        if (getOptRet != 0)
        {
            throw OsErrorException("getopt_long", errno);
        }

        if (theOpts[optIndex].has_arg)
        {
            if (nullptr == optarg)
            {
                /*
                 * If the option that "getopt_long()" found next requires an
                 * argument, and none was given (as indicated by the C
                 * library-provided global variable "optarg"), bail out. This
                 * should not happen, since when getopt_long() detects this, it
                 * returns something other than '0', precipitating an
                 * InvalidInputException as shown above.
                 */
                stringstream formattedMsg;
                formattedMsg << "getopt_long() let an command line flag "
                             << "requiring an argument pass without the argument!"
                             << ends;
                throw RuntimeErrorException(formattedMsg.str());
            }
            /*
             * Save the argument provided with the command line flag in the map
             * of values that will be accessible to the framework developer. If
             * a value was already stored for the option, append it using a ','
             * as a separator.
             */
            string optName = theOpts[optIndex].name;
            ArgValMap::iterator argVal = m_argValMap.find(optName);
            if (argVal != m_argValMap.end())
            {
                (*argVal).second += "," + string((const char*)optarg);
            }
            else
            {
                m_argValMap[optName] = string((const char*)optarg);
            }
        }
        else
        {
            /*
             * If a command line flag was present, but it does not require an
             * argument, store a non-empty string in the map of values that
             * will be accessible to the framework developer.
             */
            m_argValMap[theOpts[optIndex].name] = "T";
        }
    }

    delete [] theOpts;
}

RunLoop*
Application::getMainRunLoop() const
{
    if (nullptr == m_mainThread)
    {
        throw PreconditionNotMetException("Application not initialized.");
    }

    return m_mainThread->runLoop();
}

RunLoop*
Application::getCurrentRunLoop() const
{
    RunLoop *result = nullptr;
    pthread_t currentThread = pthread_self();
    vector<Thread*>::const_iterator currentThrObjIter = m_appThreads.end();

    if (nullptr == m_mainThread)
    {
        throw PreconditionNotMetException("Application already initialized.");
    }

    if (m_mainThread->threadId() == currentThread)
    {
        result = m_mainThread->runLoop();
    }
    else
    {
        /*
         * Search through all the subordinate Thread objects for one that
         * is associated with the thread matching the POSIX thread ID of the
         * thread that called this method.
         */
        currentThrObjIter = find_if(m_appThreads.begin(), m_appThreads.end(),
                bind2nd(ptr_fun(&matchPthreadId), currentThread));
        if (currentThrObjIter != m_appThreads.end())
        {
            result = (*currentThrObjIter)->runLoop();
        }
    }

    return result;
}


AppLog&
Application::log()
{
    return *m_log;
}

// vim: set ts=4 sw=4 expandtab:
