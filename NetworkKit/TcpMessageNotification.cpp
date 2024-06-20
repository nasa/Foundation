/**
 * \file      TcpMessageNotification.cpp
 * \brief     Definition of TcpMessageNotification class
 * \date      2012-12-13
 * \author    Ryan O'Farrell
 */
#include <iterator>

#include "TcpMessageNotification.h"

namespace NetworkKit
{

TcpMessageNotification::TcpMessageNotification(timespec const& theAcqTime,
		CoreKit::FixedByteVector const& thePayload,
		const TcpSocket * const theSocket) :
		acqTime(m_acqTime), message(m_message), socket(theSocket), m_acqTime(
				theAcqTime), m_message(thePayload)
{

}

TcpMessageNotification::TcpMessageNotification(
		TcpMessageNotification const & other) :
		acqTime(m_acqTime), message(m_message), socket(other.socket), m_acqTime(
				other.m_acqTime), m_message(other.m_message)
{
}

TcpMessageNotification::TcpMessageNotification(size_t bufferSize,
		const TcpSocket * const theSocket) :
		acqTime(m_acqTime), message(m_message), socket(theSocket), m_acqTime(), m_message(
				bufferSize)
{

}

std::ostream & operator<<(std::ostream &os, const TcpMessageNotification& p)
{
	os << "Message: ";
	std::copy(p.message.begin(), p.message.end(),
			std::ostream_iterator<uint8_t>(os));
	os << "\t Acq time: " << p.acqTime.tv_sec << "." << p.acqTime.tv_nsec;
	return os;
}

} /* namespace NetworkKit */
