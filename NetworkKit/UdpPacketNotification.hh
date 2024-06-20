/**
 * \file UdpPacketNotification.hh
 * \brief Contains the template implementations for the \c NetworkKit::UdpPacketNotification family of classes.
 * \date 2019-08-15 09:54:43
 * \author Rolando J. Nieves
 */

#ifndef _FOUNDATION_NETWORKKIT_UDPPACKETNOTIFICATION_CC_
#define _FOUNDATION_NETWORKKIT_UDPPACKETNOTIFICATION_CC_

#include <algorithm>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include "UdpPacketNotification.h"

namespace NetworkKit
{

template< typename ByteVector >
UdpPacketNotification::UdpPacketNotification(
    double in_acqTime,
    int in_addressFamily,
    ByteVector const& in_packetContents
):
    acqTime(in_acqTime),
    addressFamily(in_addressFamily),
    packetContents(UdpPacketNotification::MAX_PACKET_SIZE)
{
    packetContents.resize(in_packetContents.size());

    std::copy(
        in_packetContents.begin(),
        in_packetContents.end(),
        packetContents.begin()
    );
}


template< typename ByteVector >
UdpUxPacketNotification::UdpUxPacketNotification(
    double in_acqTime,
    std::string const& in_socketPath,
    ByteVector const& in_packetContents
):
    UdpPacketNotification(in_acqTime, AF_UNIX, in_packetContents),
    socketPath(in_socketPath)
{

}


template< typename ByteVector >
UdpIpPacketNotification::UdpIpPacketNotification(
    double in_acqTime,
    std::string const& in_ipAddress,
    int in_port,
    ByteVector const& in_packetContents
):
    UdpPacketNotification(in_acqTime, AF_INET, in_packetContents),
    ipAddress(in_ipAddress),
    port(in_port)
{

}

} // end namespace NetworkKit

#endif /* !_FOUNDATION_NETWORKKIT_UDPPACKETNOTIFICATION_CC_ */

// vim: set ts=4 sw=4 expandtab:
