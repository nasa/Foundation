/**
 * \file SignalInputSource.cpp
 * \brief Implementation of the SignalInputSource Class
 * \date 2012-09-24 08:39:57
 * \author Rolando J. Nieves
 */

#include <unistd.h>
#include <fcntl.h>
#include <sys/signalfd.h>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <stdexcept>

#include "SignalInputSource.h"
#include "OsErrorException.h"

using CoreKit::SignalInputSource;
using CoreKit::InterruptListener;
using CoreKit::OsErrorException;

SignalInputSource::SignalInputSource(int sigNum, InterruptListener *intrListener)
: m_sigNum(sigNum), m_intrListener(intrListener), m_signalFd(-1)
{
	sigset_t theSignal;

    if (nullptr == intrListener)
    {
        throw std::runtime_error("Invalid interrupt listener dependency injection.");
    }

	/*
	 * Add the signal provided as input to a brand new set that will be used
	 * to call signalfd() and pthread_sigmask().
	 */
	sigemptyset(&theSignal);
	sigaddset(&theSignal, sigNum);

	m_signalFd = signalfd(-1, &theSignal,
#if defined(HAVE_SFD_NONBLOCK) && (HAVE_SFD_NONBLOCK == 1)
		SFD_NONBLOCK
#else
		0
#endif /* defined(HAVE_SFD_NONBLOCK) && (HAVE_SFD_NONBLOCK == 1) */
		);
	if (-1 == m_signalFd)
	{
		throw OsErrorException("signalfd", errno);
	}
#if !defined(HAVE_SFD_NONBLOCK) || (HAVE_SFD_NONBLOCK == 0)
	if (fcntl(m_signalFd, F_SETFL, O_NONBLOCK) == -1)
	{
		throw OsErrorException("signal file descriptor fcntl", errno);
	}
#endif /* !defined(HAVE_SFD_NONBLOCK) || (HAVE_SFD_NONBLOCK == 0) */
	pthread_sigmask(SIG_BLOCK, &theSignal, NULL);
}


SignalInputSource::~SignalInputSource()
{
	sigset_t theSignal;

	sigemptyset(&theSignal);

	/*
	 * Close the file descriptor used to receive signals
	 */
	if (m_signalFd != -1)
	{
		close(m_signalFd);
	}

	/*
	 * Remove the signal of interest from the list of blocked signals by the
	 * application process.
	 */
	sigaddset(&theSignal, m_sigNum);
	pthread_sigmask(SIG_UNBLOCK, &theSignal, NULL);
}





int SignalInputSource::fileDescriptor() const
{
	return m_signalFd;
}


InterruptListener* SignalInputSource::interruptListener() const
{
	return  m_intrListener;
}


void SignalInputSource::fireCallback()
{
	struct signalfd_siginfo aSigInfo;
	ssize_t readResult = 0;

	/*
	 * signalfd() will publish via the file descriptor instances of the
	 * signalfd_siginfo data structure whenever the application process
	 * receives the signal of interest. See the signalfd() man page for more
	 * details.
	 */
	do
	{
		readResult = read(m_signalFd, &aSigInfo, sizeof(aSigInfo));
	} while (sizeof(aSigInfo) == readResult);

	/*
	 * Call the InterruptListener to let them execute any custom logic in
	 * response to the received signal.
	 */
	m_intrListener->signalReceived(m_sigNum);
}
