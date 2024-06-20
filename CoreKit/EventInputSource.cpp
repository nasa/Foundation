/**
 * \file EventInputSource.cpp
 * \brief Contains the implementation of the \c EventInputSource class.
 * \date 2019-05-15 16:49:11
 * \author Rolando J. Nieves
 */

#include <cerrno>
#include <stdexcept>
#include <sys/eventfd.h>
#include <unistd.h>

#include <CoreKit/OsErrorException.h>

#include "EventInputSource.h"


namespace CoreKit
{

EventInputSource::EventInputSource(InterruptListener *intrListener):
    m_eventFd(-1),
    m_intrListener(intrListener)
{
    if (nullptr == intrListener)
    {
        throw std::runtime_error("Invalid interrupt listener dependency injected.");
    }
    
    m_eventFd = eventfd(0uLL, EFD_CLOEXEC | EFD_NONBLOCK);
    if (-1 == m_eventFd)
    {
        throw OsErrorException("eventfd()", errno);
    }
}


EventInputSource::~EventInputSource()
{
    if (m_eventFd != -1)
    {
        close(m_eventFd);
        m_eventFd = -1;
    }
}


int
EventInputSource::fileDescriptor() const
{
    return m_eventFd;
}


InterruptListener*
EventInputSource::interruptListener() const
{
    return m_intrListener;
}


void
EventInputSource::fireCallback()
{
    if (m_eventFd != -1)
    {
        eventfd_t readValue = 0uLL;
        int readResult = eventfd_read(m_eventFd, &readValue);
        m_intrListener->inputAvailableFrom(this);
    }
}


void
EventInputSource::assertEvent()
{
    if (m_eventFd != -1)
    {
        eventfd_write(m_eventFd, 1uLL);
    }
}

} // end namespace CoreKit

// vim: set ts=4 sw=4 expandtab:
