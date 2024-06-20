/**
 * \file CanBusIo.cpp
 * \brief Implementation of the CanBusIo Class
 * \date 2012-09-21 09:01:54
 * \author Rolando J. Nieves
 */

#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can/raw.h>
#include <fcntl.h>
#include <ctime>
#include <cerrno>
#include <cstring>
#include <algorithm>
#include <functional>

#include <CoreKit/PreconditionNotMetException.h>
#include <CoreKit/OsErrorException.h>
#include <CoreKit/InvalidInputException.h>
#include <CoreKit/AppLog.h>

#include "CanBusIo.h"

#define CBK_MAX_FRAME_READ_COUNT (10)

using std::copy;
using std::for_each;
using std::vector;
using std::bind2nd;
using std::ptr_fun;
using std::mem_fun;
using CoreKit::InterruptListener;
using CoreKit::InputSource;
using CoreKit::RunLoop;
using CoreKit::PreconditionNotMetException;
using CoreKit::OsErrorException;
using CoreKit::InvalidInputException;
using CoreKit::format;

namespace CanBusKit {

static void deleteCanBusCallback(CanBusFrameCallback *aCallback)
{
	delete aCallback;
}


CanBusIo::CanBusIo(std::string const& canIfName, RunLoop* theRunLoop)
: m_canBusIfName(canIfName), m_canBusIfIndex(-1), m_canFilterCount(0u), m_canBusFd(-1),
  m_runLoop(theRunLoop), m_canIfState(CREATED)
{
	memset(m_canFilters, 0x00, sizeof(m_canFilters));
}


CanBusIo::CanBusIo(std::string const& canIfName, RunLoop* theRunLoop, std::vector<struct can_filter> const& inputFilter)
: m_canBusIfName(canIfName), m_canBusIfIndex(-1), m_canFilterCount(0u), m_canBusFd(-1),
  m_runLoop(theRunLoop), m_canIfState(CREATED)
{
	memset(m_canFilters, 0x00, sizeof(m_canFilters));
	if (inputFilter.size() > RF_CBK_MAX_FILTER_COUNT)
	{
		throw InvalidInputException("CAN Bus Filter Count Too Large",
				format("%d", (int)inputFilter.size()));
	}

	copy(inputFilter.begin(), inputFilter.end(), &m_canFilters[0]);
	m_canFilterCount = inputFilter.size();
}



CanBusIo::~CanBusIo()
{
	for_each(m_callbacks.begin(), m_callbacks.end(), &deleteCanBusCallback);
	m_callbacks.clear();
}






void CanBusIo::addCanFrameCallback(CanBusFrameCallback* theCallback)
{
	m_callbacks.push_back(theCallback);
}


void CanBusIo::decipherCanBusIfIndex()
{
	struct ifreq canIfReq;

	strncpy(canIfReq.ifr_name, m_canBusIfName.c_str(), sizeof(canIfReq.ifr_name));
	if (ioctl(m_canBusFd, SIOCGIFINDEX, &canIfReq) == -1)
	{
		throw OsErrorException("ioctl", errno);
	}
	m_canBusIfIndex = canIfReq.ifr_ifindex;
}


int CanBusIo::fileDescriptor() const
{
	return m_canBusFd;
}


InterruptListener* CanBusIo::interruptListener() const
{
	return const_cast<CanBusIo*>(this);
}


void CanBusIo::inputAvailableFrom(InputSource* theInputSource)
{
	ssize_t readResult = -1;
	struct can_frame aFrame[CBK_MAX_FRAME_READ_COUNT];
	size_t readCount = 0u;
	unsigned cbIdx = 0u;

	/*
	 * Read at most ten (10) frames from the CAN Bus device.
	 */
	do
	{
		readResult = read(m_canBusFd, &aFrame[readCount], sizeof(struct can_frame));
		if (sizeof(struct can_frame) == readResult)
		{
			readCount++;
		}
	} while ((sizeof(struct can_frame) == readResult) && (readCount < CBK_MAX_FRAME_READ_COUNT));

	/*
	 * Dispatch a callback to every listener for each CAN Bus frame received.
	 */
	for (cbIdx = 0u; cbIdx < readCount; cbIdx++)
	{
		m_prototypeNotif.m_canId = aFrame[cbIdx].can_id;
		m_prototypeNotif.decodeCanId();
		m_prototypeNotif.m_canPayload.resize(aFrame[cbIdx].can_dlc, 0x00);
		copy(&aFrame[cbIdx].data[0], &aFrame[cbIdx].data[aFrame[cbIdx].can_dlc], m_prototypeNotif.m_canPayload.begin());
		clock_gettime(CLOCK_REALTIME, &m_prototypeNotif.m_acqTime);
		for_each(m_callbacks.begin(), m_callbacks.end(),
				bind2nd(mem_fun(&CanBusFrameCallback::operator()), &m_prototypeNotif));
	}
}


void CanBusIo::fireCallback()
{
	this->inputAvailableFrom(this);
}


void CanBusIo::sendCanFrame(struct can_frame* theFrame)
{
	if (-1 == m_canBusFd)
	{
		throw PreconditionNotMetException("CAN Bus Interface Already Configured");
	}

	if (theFrame->can_id > 0x7FF)
	{
		theFrame->can_id |= CAN_EFF_FLAG;
	}
	
	if (write(m_canBusFd, theFrame, sizeof(struct can_frame)) != sizeof(struct can_frame))
	{
		// TODO: Report warning
	}
}


void CanBusIo::startCan()
{
	struct sockaddr_can canAddr;

	memset(&canAddr, 0x00, sizeof(canAddr));

	if (m_canBusFd != -1)
	{
		throw PreconditionNotMetException("CAN Bus Interface Not Configured");
	}

#if defined(HAVE_SOCK_NONBLOCK) && (HAVE_SOCK_NONBLOCK == 1)
	m_canBusFd = socket(PF_CAN, SOCK_RAW | SOCK_NONBLOCK, CAN_RAW);
#else
	m_canBusFd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
#endif /* defined(HAVE_SOCK_NONBLOCK) && (HAVE_SOCK_NONBLOCK == 1) */
	if (m_canBusFd < 0)
	{
		throw OsErrorException("socket", errno);
	}

#if !defined(HAVE_SOCK_NONBLOCK) || (HAVE_SOCK_NONBLOCK != 1)
	if (fcntl(m_canBusFd, F_SETFL, O_NONBLOCK) == -1)
	{
		throw OsErrorException("fcntl", errno);
	}
#endif /* !defined(HAVE_SOCK_NONBLOCK) || (HAVE_SOCK_NONBLOCK != 1) */
	this->decipherCanBusIfIndex();

	canAddr.can_family = PF_CAN;
	canAddr.can_ifindex = m_canBusIfIndex;
	if (bind(m_canBusFd, reinterpret_cast<struct sockaddr*>(&canAddr), sizeof(canAddr)) == -1)
	{
		throw OsErrorException("bind", errno);
	}

	if (m_canFilterCount > 0u)
	{
		if (setsockopt(m_canBusFd, SOL_CAN_RAW, CAN_RAW_FILTER, &m_canFilters,
				(m_canFilterCount * sizeof(struct can_filter))) == -1)
		{
			throw OsErrorException("setsockopt", errno);
		}
	}

	m_canIfState = STARTED;

	if (m_runLoop != NULL)
	{
		m_runLoop->registerInputSource(this);
		m_canIfState = REGISTERED;
	}
}


void CanBusIo::stopCan()
{
	if (m_canBusFd != -1)
	{
		if ((m_runLoop != NULL) && (REGISTERED == m_canIfState))
		{
			m_runLoop->deregisterInputSource(this);
		}

		close(m_canBusFd);
		m_canBusFd = -1;
		m_canIfState = CREATED;
	}
}

} // namespace CanBusKit
