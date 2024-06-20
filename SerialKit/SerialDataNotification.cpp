/**
 * \file SerialDataNotification.cpp
 * \brief Implementation of the SerialDataNotification Class
 * \date 2012-09-20 17:26:00
 * \author Rolando J. Nieves
 */

#include "SerialDataNotification.h"

using CoreKit::FixedByteVector;
using SerialKit::SerialDataNotification;


namespace SerialKit
{

SerialDataNotification::SerialDataNotification(std::string const& theSerialPort, timespec const& theAcqTime, FixedByteVector const& theSerialData)
: serialPort(m_serialPort), m_serialPort(theSerialPort),
  acqTime(m_acqTime), m_acqTime(theAcqTime),
  serialData(m_serialData), m_serialData(theSerialData)
{

}

SerialDataNotification::SerialDataNotification(const SerialDataNotification &other)
: serialPort(m_serialPort), m_serialPort(other.m_serialPort),
  acqTime(m_acqTime), m_acqTime(other.m_acqTime),
  serialData(m_serialData), m_serialData(other.m_serialData)
{

}


SerialDataNotification::SerialDataNotification(size_t bufferSize)
: serialPort(m_serialPort),
  acqTime(m_acqTime),
  serialData(m_serialData), m_serialData(bufferSize)
{

}


std::ostream & operator<<(std::ostream &os, const SerialDataNotification& p)
{
   os << "Port : " << p.serialPort << "\t Acq time: " << p.acqTime.tv_sec << "." << p.acqTime.tv_nsec << "\n";
   for (int i = 0; i < p.serialData.size(); i++)
   {
      os << p.serialData.at(i) << "\t";
   }
   return os;
}

}
