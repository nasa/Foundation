/**
 * \file UdpSocket.hh
 * \brief Contains the template implementations for the \c NetworkKit::UdpSocket class.
 * \date 2019-08-15 12:19:44
 * \author Rolando J. Nieves
 */

#ifndef _FOUNDATION_NETWORKKIT_UDPSOCKET_CC_
#define _FOUNDATION_NETWORKKIT_UDPSOCKET_CC_

#include <algorithm>
#include <arpa/inet.h>

#include "UdpSocket.h"


namespace NetworkKit
{

template< typename ByteVector >
ssize_t
UdpSocket::sendTo(std::string const& uxPath, ByteVector const& packetContents)
{
    if (m_selectedFamily != AF_UNIX)
    {
        return -1;
    }

    struct sockaddr_un destAddr;

    memset(&destAddr, 0x00, sizeof(destAddr));
    destAddr.sun_family = AF_UNIX;
    strncpy(&destAddr.sun_path[0], uxPath.c_str(), sizeof(destAddr.sun_path) - 1u);
    ssize_t result = sendto(
        m_socketFd,
        packetContents.data(),
        packetContents.size(),
        0,
        reinterpret_cast< struct sockaddr* >(&destAddr),
        sizeof(sockaddr_un)
    );

    return result;
}


template< typename ByteVector >
ssize_t
UdpSocket::sendTo(std::string const& ipAddr, int port, ByteVector const& packetContents)
{
    if (m_selectedFamily != AF_INET)
    {
        return -1;
    }

    struct sockaddr_in destAddr;

    memset(&destAddr, 0x00, sizeof(destAddr));
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(port);
    inet_pton(AF_INET, ipAddr.c_str(), &destAddr.sin_addr);

    ssize_t result = sendto(
        m_socketFd,
        packetContents.data(),
        packetContents.size(),
        0,
        reinterpret_cast< struct sockaddr* >(&destAddr),
        sizeof(destAddr)
    );

    return result;
}


template< typename ByteVector >
std::size_t
UdpSocket::receiveFrom(std::string& uxPath, ByteVector& packetContents)
{
    if (m_selectedFamily != AF_UNIX)
    {
        return 0u;
    }

    struct sockaddr_un fromAddr;
    socklen_t fromAddrLen = sizeof(fromAddr);

    memset(&fromAddr, 0x00, sizeof(fromAddr));

    ssize_t result = recvfrom(
        m_socketFd,
        packetContents.data(),
        packetContents.size(),
        MSG_DONTWAIT,
        reinterpret_cast< struct sockaddr* >(&fromAddr),
        &fromAddrLen
    );

    if
    (
        (result >= 0) &&
        (fromAddr.sun_family == AF_UNIX) &&
        (fromAddrLen > sizeof(sa_family_t))
    )
    {
        std::size_t pathSize = fromAddrLen - sizeof(sa_family_t) - 1;
        pathSize = std::min(pathSize, sizeof(fromAddr.sun_path) - 1);
        uxPath.resize(pathSize);
        std::copy_n(
            &fromAddr.sun_path[0],
            pathSize,
            uxPath.begin()
        );
    }

    return static_cast< std::size_t >(
        std::max(
            result,
            static_cast< ssize_t >(0)
        )
    );
}


template< typename ByteVector >
std::size_t
UdpSocket::receiveFrom(std::string& ipAddr, int& port, ByteVector& packetContents)
{
    if (m_selectedFamily != AF_INET)
    {
        return 0u;
    }
    
    struct sockaddr_in fromAddr;
    socklen_t fromAddrLen = sizeof(fromAddr);
    char addrArea[INET_ADDRSTRLEN];

    memset(&fromAddr, 0x00, sizeof(fromAddr));
    memset(&addrArea[0], 0x00, INET_ADDRSTRLEN);

    ssize_t result = recvfrom(
        m_socketFd,
        packetContents.data(),
        packetContents.size(),
        MSG_DONTWAIT,
        reinterpret_cast< struct sockaddr* >(&fromAddr),
        &fromAddrLen
    );

    if ((result >= 0) && (fromAddr.sin_family == AF_INET))
    {
        ipAddr = inet_ntop(
            AF_INET,
            &fromAddr.sin_addr,
            addrArea,
            INET_ADDRSTRLEN
        );
        port = ntohs(fromAddr.sin_port);
    }

    return static_cast< std::size_t >(
        std::max(
            result,
            static_cast< ssize_t >(0)
        )
    );
}

} // end namespace NetworkKit

#endif /* !_FOUNDATION_NETWORKKIT_UDPSOCKET_CC_ */

// vim: set ts=4 sw=4 expandtab:
