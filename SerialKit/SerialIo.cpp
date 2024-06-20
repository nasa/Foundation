/**
 * \file SerialIo.cpp
 * \brief Implementation of the SerialIo Class
 * \date 20-Sep-2012 5:26:00 PM
 * \author rnieves
 * \copyright National Aeronautics and Space Administration
 */

#if defined(HAVE_CONFIG_H)
# include "config_foundation.h"
#endif /* defined(HAVE_CONFIG_H) */

#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <cstring>
#include <cerrno>
#include <cmath>
#include <fcntl.h>
#include <functional>
#include <algorithm>

#include "CoreKit/PreconditionNotMetException.h"
#include "CoreKit/OsErrorException.h"
#include "CoreKit/AppLog.h"
#include "SerialIo.h"

using std::string;
using std::for_each;
using std::ptr_fun;
using std::mem_fun;
using std::bind2nd;
using CoreKit::InterruptListener;
using CoreKit::RunLoop;
using CoreKit::AppLog;
using CoreKit::PreconditionNotMetException;
using CoreKit::OsErrorException;

namespace SerialKit {

/**
 * \brief Deletes a single callback object
 * \param aCallbackObj to be deleted
 */
static void deleteCallbackObj(SerialDataCallback *aCallbackObj)
{
	delete aCallbackObj;
}


static struct timeval& timevalFromFloat(struct timeval& tv, float tvAsFloat)
{
	float fracPart = fmodf(tvAsFloat, 1.0f);
	tv.tv_sec = (time_t)tvAsFloat;
	tv.tv_usec = (suseconds_t)(fracPart * 1000000.0f);

	return tv;
}


static bool hasTimeLeft(struct timeval const& tv)
{
	return tv.tv_sec > 0 || tv.tv_usec > 0;
}


SerialIo::SerialIo(string const& serialPort, RunLoop* hostRunLoop, AppLog *i_log)
: m_runLoop(hostRunLoop), m_log(i_log), m_serialPortFd(-1), m_serialPort(serialPort),
  m_prototypeNotif(NULL), m_buffering(false)
{
	m_prototypeNotif = new SerialDataNotification(RF_SIO_DATA_CHUNK_MAX_SIZE);
    m_prototypeNotif->m_serialPort = m_serialPort;
}



SerialIo::~SerialIo()
{
    try
    {
        if (NULL != m_runLoop)
        {
            m_runLoop->deregisterInputSource(this);
        }
    }
    catch(CoreKit::OsErrorException &osError)
    {
        if (NULL != m_log)
        {
            *m_log << AppLog::LL_DEBUG << "Error de-registering input source " << osError.what() << CoreKit::EndLog;
        }
    }
	this->closeSerialPort();
	for_each(m_callbacks.begin(), m_callbacks.end(), ptr_fun(&deleteCallbackObj));
	m_callbacks.clear();
	delete m_prototypeNotif;
	m_prototypeNotif = NULL;
}



void SerialIo::closeSerialPort()
{
	if (m_serialPortFd != -1)
	{
		close(m_serialPortFd);
		m_serialPortFd = -1;
	}
}


int SerialIo::fileDescriptor() const
{
	return m_serialPortFd;
}


InterruptListener* SerialIo::interruptListener() const
{
	return const_cast<SerialIo*>(this);
}


void SerialIo::inputAvailableFrom(InputSource* theInputSource)
{
	if (m_prototypeNotif->serialData.empty())
	{
		clock_gettime(CLOCK_REALTIME, &m_prototypeNotif->m_acqTime);
	}
	this->readAvailableData(m_prototypeNotif->m_serialData);
	if (!m_buffering || (m_prototypeNotif->serialData.size() == m_prototypeNotif->serialData.max_size()))
	{
		for_each(m_callbacks.begin(), m_callbacks.end(),
				bind2nd(mem_fun(&SerialDataCallback::operator()), m_prototypeNotif));
		m_prototypeNotif->m_serialData.clear();
	}
}


void SerialIo::openSerialPort()
{
	if (m_serialPortFd != -1)
	{
		throw PreconditionNotMetException("Serial port is already open.");
	}

	m_serialPortFd = open(m_serialPort.c_str(), O_RDWR | O_NONBLOCK);
	if (-1 == m_serialPortFd)
	{
		throw OsErrorException("open", errno);
	}
}


void SerialIo::registerSerialDataCallback(SerialDataCallback* theCallback)
{
    if (NULL == theCallback)
    {
        throw CoreKit::PreconditionNotMetException("Can not register NULL callback");
    }
	m_callbacks.push_back(theCallback);
}


void SerialIo::startSerialIo()
{
	this->openSerialPort();

	if (NULL != m_runLoop)
	{
		m_runLoop->registerInputSource(this);
	}
}

void SerialIo::startSerialIo(speed_t baudRate, bool raw)
{
	this->startSerialIo();

	//configure serial port settings
	struct termios tio;
	memset(&tio, 0, sizeof(tio));


	// store current settings in tio
	if (tcgetattr(m_serialPortFd,&tio) < 0)
	{
		throw CoreKit::OsErrorException("Cannot get TTY settings.", errno);
	}

    if (cfsetospeed(&tio, baudRate) < 0)
    {
        throw CoreKit::OsErrorException("Cannot set output baud rate", errno);
    }

    if (cfsetispeed(&tio, baudRate) < 0)
    {
        throw CoreKit::OsErrorException("Cannot set input baud rate", errno);
    }

	if (raw)
	{
        //make serial port use "raw" settings
        cfmakeraw(&tio);

        tio.c_cflag &= ~IMAXBEL;
        tio.c_lflag &= ~ECHOE;
        tio.c_lflag &= ~ECHOK;
        tio.c_lflag &= ~ECHOCTL;
        tio.c_lflag &= ~ECHOKE;
	}

    if (tcsetattr(m_serialPortFd, TCSANOW, &tio) < 0)
    {
        throw CoreKit::OsErrorException("Cannot set TTY settings.", errno);
    }
}

void SerialIo::stopSerialIo()
{
	if (NULL != m_runLoop)
	{
	    try
	    {
            m_runLoop->deregisterInputSource(this);
        }
        catch(CoreKit::OsErrorException &osError)
        {
            if (NULL != m_log)
            {
                *m_log << AppLog::LL_DEBUG << "Error de-registering input source " << osError.what() << CoreKit::EndLog;
            }
        }
	}

	this->closeSerialPort();
}


void SerialIo::fireCallback()
{
	this->inputAvailableFrom(this);
}


void SerialIo::bufferData(size_t bufferSize)
{
	SerialDataNotification *newNotif = NULL;

	if (0 == bufferSize)
	{
		m_buffering = false;
		bufferSize = RF_SIO_DATA_CHUNK_MAX_SIZE;
	}
	else
	{
		m_buffering = true;
	}

	if (bufferSize != m_prototypeNotif->serialData.max_size())
	{
		newNotif = new SerialDataNotification(bufferSize);
		delete m_prototypeNotif;
		m_prototypeNotif = newNotif;
	}
}


int SerialIo::writeWithBlock(uint8_t const* data, size_t size, float timeout)
{
	static const size_t INTERRUPT_COUNT_TOLERANCE = 3u;
	struct timeval selectTimeout;
	fd_set outputFd;
	int selectResult = 0;
	ssize_t writeResult = 0;
	size_t writtenCount = 0u;
	int result = 0;
	size_t interruptCount = 0u;

	FD_ZERO(&outputFd);
	memset(&selectTimeout, 0x00, sizeof(selectTimeout));

	timevalFromFloat(selectTimeout, timeout);

	while (hasTimeLeft(selectTimeout) && (writtenCount < size))
	{
		FD_ZERO(&outputFd);
		FD_SET(m_serialPortFd, &outputFd);
		//
		// Counting on Linux-specific behavior where the select() call modifies
		// the time left to wait once output capacity was available.
		//
		selectResult = select(m_serialPortFd + 1, NULL, &outputFd, NULL, &selectTimeout);
		if (selectResult > 0)
		{
			//
			// Not doing the FD_ISSET() check because the serial port output FD
			// was the only FD in the set.
			//
			writeResult = write(m_serialPortFd, &data[writtenCount], (size - writtenCount));
			if (writeResult > 0)
			{
				writtenCount += writeResult;
			}
			else if (writeResult < 0)
			{
				if (EINTR == errno)
				{
					if (++interruptCount > INTERRUPT_COUNT_TOLERANCE)
					{
						result = errno;
						break;
					}
				}
				else
				{
					result = errno;
					break;
				}
			}
		}
		else if (selectResult < 0)
		{
			result = errno;
			break;
		}
		result = static_cast<int>(writtenCount);
	}

	return result;
}

} // namespace SerialKit
