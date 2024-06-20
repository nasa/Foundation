/**
 * \file UdpPacketNotification.cpp
 * \brief Contains the implementation of the \c NetworkKit::UdpPacketNotification class family.
 * \date 2019-08-15 09:35:40
 * \author Rolando J. Nieves
 */

#include <cmath>
#include <limits>
#include <algorithm>

#include "UdpPacketNotification.hh"

namespace NetworkKit
{

UdpPacketNotification::UdpPacketNotification():
    acqTime(std::numeric_limits< double >::quiet_NaN()),
    addressFamily(-1),
    packetContents(UdpPacketNotification::MAX_PACKET_SIZE)
{

}


UdpPacketNotification::UdpPacketNotification(UdpPacketNotification const& other):
    acqTime(other.acqTime),
    addressFamily(other.addressFamily),
    packetContents(UdpPacketNotification::MAX_PACKET_SIZE)
{
    packetContents.resize(other.packetContents.size());
    std::copy(
        other.packetContents.begin(),
        other.packetContents.end(),
        packetContents.begin()
    );
}


bool
UdpPacketNotification::isValid() const
{
    return std::isfinite(acqTime) && (addressFamily > 0);
}


UdpPacketNotification&
UdpPacketNotification::operator=(UdpPacketNotification const& other)
{
    acqTime = other.acqTime;
    addressFamily = other.addressFamily;
    packetContents.resize(other.packetContents.size());
    std::copy(
        other.packetContents.begin(),
        other.packetContents.end(),
        packetContents.begin()
    );

    return *this;
}


UdpUxPacketNotification::UdpUxPacketNotification():
    UdpPacketNotification()
{
    this->addressFamily = AF_UNIX;
}


UdpUxPacketNotification::UdpUxPacketNotification(UdpUxPacketNotification const& other):
    UdpPacketNotification(other),
    socketPath(other.socketPath)
{

}


bool
UdpUxPacketNotification::isValid() const
{
    return UdpPacketNotification::isValid() && !socketPath.empty();
}


UdpUxPacketNotification&
UdpUxPacketNotification::operator=(UdpUxPacketNotification const& other)
{
    UdpPacketNotification::operator=(other);

    socketPath = other.socketPath;

    return *this;
}


UdpIpPacketNotification::UdpIpPacketNotification():
    UdpPacketNotification(),
    port(-1)
{
    this->addressFamily = AF_INET;
}


UdpIpPacketNotification::UdpIpPacketNotification(UdpIpPacketNotification const& other):
    UdpPacketNotification(other),
    ipAddress(other.ipAddress),
    port(other.port)
{

}


bool
UdpIpPacketNotification::isValid() const
{
    return UdpPacketNotification::isValid() && !ipAddress.empty() && (port > 0);
}


UdpIpPacketNotification&
UdpIpPacketNotification::operator=(UdpIpPacketNotification const& other)
{
    UdpPacketNotification::operator=(other);

    ipAddress = other.ipAddress;
    port = other.port;

    return *this;
}

} // end namespace NetworkKit

// vim: set ts=4 sw=4 expandtab:
