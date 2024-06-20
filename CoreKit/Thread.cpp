/**
 * \file Thread.cpp
 * \brief Implementation of the Thread Class
 * \date 2012-09-24 08:39:58
 * \author Rolando J. Nieves
 */

#include <pthread.h>
#include <cstdlib>
#include <cstring>
#include <cerrno>

#include <CoreKit/Application.h>
#include <CoreKit/OsErrorException.h>
#include <CoreKit/SynchronizedRunLoop.h>

#include "Thread.h"

using CoreKit::Thread;
using CoreKit::RunLoop;
using CoreKit::ThreadDelegate;
using CoreKit::OsErrorException;

/**
 * \brief Common Entry Point for POSIX Threads
 *
 * The \c threadKickoffRoutime() local function is used as the entry point
 * for all POSIX threads spawned by \c Thread instances.
 *
 * \param userData Pointer to the \c Thread object that spawned the thread.
 *
 * \return Always \c nullptr
 */
void* Thread::threadKickoffRoutine(void *userData)
{
	Thread *theThread = (Thread *)userData;

	theThread->threadDelegate()->doThreadLogic(theThread);

    theThread->m_threadState = Thread::STOPPED;

	return nullptr;
}

Thread::Thread(pthread_t runningThreadId)
    : m_threadDelegate(nullptr),m_runLoop(nullptr),m_threadState(Thread::RUNNING),m_threadId(runningThreadId),
      m_isDetached(true), m_hostApp(nullptr)
{
	m_runLoop = new RunLoop();
	m_runLoop->setHostThread(this);
}


Thread::Thread(pthread_t runningThreadId, sem_t *runLoopSyncObj)
: m_threadDelegate(nullptr), m_runLoop(nullptr), m_threadState(Thread::RUNNING), m_threadId(runningThreadId),
  m_isDetached(true), m_hostApp(nullptr)
{
	m_runLoop = new SynchronizedRunLoop(runLoopSyncObj);
	m_runLoop->setHostThread(this);
}


Thread::Thread(ThreadDelegate* thrDelegate, Application *hostApp, bool detached)
    : m_threadDelegate(thrDelegate),m_runLoop(nullptr),m_threadState(Thread::JOINED),m_threadId(0),
      m_isDetached(detached), m_hostApp(hostApp)
{
	m_runLoop = construct(RunLoop::myType());
	m_runLoop->setHostThread(this);

	if (pthread_create(&m_threadId, nullptr, &Thread::threadKickoffRoutine, this) == -1)
	{
		throw OsErrorException("pthread_create", errno);
	}

	if (m_isDetached)
	{
		pthread_detach(m_threadId);
	}

	m_threadState = Thread::RUNNING;
}


Thread::~Thread()
{
	destroy(m_runLoop);
	m_runLoop = nullptr;
}


void Thread::join()
{
	if (false == m_isDetached)
	{
		pthread_join(m_threadId, nullptr);
        m_threadState = Thread::JOINED;
	}
}
