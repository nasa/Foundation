/**
 * \file BaseDdsTopic.cpp
 * \brief Implementation of the BaseDdsTopic Class
 * \date 2012-09-26 17:15:19
 * \author Rolando J. Nieves
 */

#include <fcntl.h>
#include <unistd.h>
#include <sys/eventfd.h>

#include <cerrno>
#include <cstdint>
#include <stdexcept>

#include <CoreKit/OsErrorException.h>
#include <CoreKit/PreconditionNotMetException.h>

#include "BaseDdsTopic.h"


using std::runtime_error;
using std::string;
using CoreKit::RunLoop;
using CoreKit::InterruptListener;
using CoreKit::PreconditionNotMetException;
using CoreKit::OsErrorException;
using DdsKit::BaseDdsTopic;

BaseDdsTopic::BaseDdsTopic(
    CoreKit::RunLoop* runLoop,
    string const& topicName
):
    m_eventFd(-1),
    m_runLoop(runLoop),
    m_topicName(topicName)
{
    if (nullptr == runLoop)
    {
        throw std::runtime_error("BaseDdsTopic construction: RunLoop instance is null.");
    }
}


BaseDdsTopic::~BaseDdsTopic()
{
	this->stopListening();
}





int BaseDdsTopic::fileDescriptor() const
{
	return m_eventFd;
}


InterruptListener* BaseDdsTopic::interruptListener() const
{
	return const_cast<BaseDdsTopic*>(this);
}


void BaseDdsTopic::inputAvailableFrom(CoreKit::InputSource *theSource)
{
	uint64_t eventVal = 0uLL;
	ssize_t readResult = 0u;

	if ((m_eventFd != -1) && (this == theSource))
	{
		readResult = read(m_eventFd, &eventVal, sizeof(eventVal));
		if (readResult != sizeof(eventVal))
		{
			// TODO: Log warning.
		}
	}
}


void BaseDdsTopic::listenForSamples()
{
	if (m_eventFd != -1)
	{
		throw PreconditionNotMetException("eventfd() Not Previously Configured");
	}

	m_eventFd = eventfd(0,
#if defined(HAVE_EFD_NONBLOCK) && (HAVE_EFD_NONBLOCK == 1)
		EFD_NONBLOCK
#else
		0
#endif /* defined(HAVE_EFD_NONBLOCK) && (HAVE_EFD_NONBLOCK == 1) */
		);
	if (-1 == m_eventFd)
	{
		throw OsErrorException("eventfd", errno);
	}
#if !defined(HAVE_EFD_NONBLOCK) || (HAVE_EFD_NONBLOCK == 0)
	if (fcntl(m_eventFd, F_SETFL, O_NONBLOCK) == -1)
	{
		throw OsErrorException("event file descriptor fcntl", errno);
	}
#endif /* !defined(HAVE_EFD_NONBLOCK) || (HAVE_EFD_NONBLOCK == 0) */

	m_runLoop->registerInputSource(this);
}


void BaseDdsTopic::stopListening()
{
	if (m_eventFd != -1)
	{
		m_runLoop->deregisterInputSource(this);
		close(m_eventFd);
		m_eventFd = -1;
	}
}


size_t BaseDdsTopic::querySampleLostCount()
{
	return 0u;
}


void BaseDdsTopic::indicateInput()
{
	uint64_t addedVal = 1uLL;
	ssize_t writeResult = -1;

	if (-1 == m_eventFd)
	{
		throw PreconditionNotMetException("eventfd() Already Configured");
	}

	writeResult = write(m_eventFd, &addedVal, sizeof(addedVal));
	if (writeResult != sizeof(addedVal))
	{
		// TODO: Log warning
	}
}
