/**
 * \file UdpPacketDistribution.cpp
 * \brief Contains the implementation of the \c NetworkKit::UdpPacketDistribution class.
 * \date 2019-08-15 10:06:28
 * \author Rolando J. Nieves
 */

#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include <CoreKit/CoreKit.h>
#include <UdpSocket.hh>

#include "UdpPacketDistribution.hh"


using CoreKit::InputSource;
using CoreKit::SystemTime;

namespace NetworkKit
{

std::atomic_ulong UdpPacketDistribution::NextCallbackId;

void
UdpPacketDistribution::readInetPacket(UdpSocket *theSocket)
{
    UdpIpPacketNotification *notif = nullptr;

    if (!m_notificationObj)
    {
        m_notificationObj.reset(
            new UdpIpPacketNotification()
        );
    }

    notif = static_cast< UdpIpPacketNotification* >(m_notificationObj.get());

    notif->packetContents.resize(UdpPacketNotification::MAX_PACKET_SIZE);

    std::size_t actualSize = theSocket->receiveFrom(
        notif->ipAddress,
        notif->port,
        notif->packetContents
    );

    notif->packetContents.resize(actualSize);
    notif->acqTime = SystemTime::now();
}


void
UdpPacketDistribution::readUxPacket(UdpSocket *theSocket)
{
    UdpUxPacketNotification *notif = nullptr;

    if (!m_notificationObj)
    {
        m_notificationObj.reset(
            new UdpUxPacketNotification()
        );
    }

    notif = static_cast< UdpUxPacketNotification* >(m_notificationObj.get());

    notif->packetContents.resize(UdpPacketNotification::MAX_PACKET_SIZE);

    std::size_t actualSize = theSocket->receiveFrom(
        notif->socketPath,
        notif->packetContents
    );

    notif->packetContents.resize(actualSize);
    notif->acqTime = SystemTime::now();
}


UdpPacketDistribution::~UdpPacketDistribution()
{
    m_callableMap.clear();
    m_notificationObj.reset();
}


void
UdpPacketDistribution::removeNotificationCallback(unsigned long callableId)
{
    m_callableMap.erase(callableId);
}


void
UdpPacketDistribution::inputAvailableFrom(InputSource *source)
{
    UdpSocket *udpSocket = dynamic_cast< UdpSocket* >(source);
    
    if (nullptr == udpSocket)
    {
        return;
    }

    if (udpSocket->selectedFamily() == AF_INET)
    {
        this->readInetPacket(udpSocket);
    }
    else if (udpSocket->selectedFamily() == AF_UNIX)
    {
        this->readUxPacket(udpSocket);
    }

    for (auto const& aCallable : m_callableMap)
    {
        aCallable.second(*m_notificationObj);
    }
}


} // end namespace NetworkKit

// vim: set ts=4 sw=4 expandtab:
