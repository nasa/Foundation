/**
 * \file CanBusFrameNotification.cpp
 * \brief Implementation of the CanBusFrameNotification Class
 * \date 2012-09-21 09:01:54
 * \author Rolando J. Nieves
 */

#include <sys/socket.h>
#include <linux/can.h>

#include "CanBusFrameNotification.h"

namespace CanBusKit {

CanBusFrameNotification::CanBusFrameNotification(uint32_t theCanId, timespec const& theAcqTime, CanPayloadVector const& thePayload)
: canId(m_canId), m_canId(theCanId),
  acqTime(m_acqTime), m_acqTime(theAcqTime),
  canPayload(m_canPayload), m_canPayload(thePayload),
  m_effMessage(false), m_rtrMessage(false), m_errFrame(false)
{
	this->decodeCanId();
}


CanBusFrameNotification::CanBusFrameNotification()
: canId(m_canId),
  acqTime(m_acqTime),
  canPayload(m_canPayload),
  m_effMessage(false), m_rtrMessage(false), m_errFrame(false)
{

}



void CanBusFrameNotification::decodeCanId()
{
	if (m_canId & CAN_EFF_FLAG)
	{
		m_effMessage = true;
		m_canId = m_canId & ~CAN_EFF_FLAG;
	}
	
	if (m_canId & CAN_RTR_FLAG)
	{
		m_rtrMessage = true;
		m_canId = m_canId & ~CAN_RTR_FLAG;
	}
	
	if (m_canId & CAN_ERR_FLAG)
	{
		m_errFrame = true;
		m_canId = m_canId & ~CAN_ERR_FLAG;
	}
}

} // namespace CanBusKit
