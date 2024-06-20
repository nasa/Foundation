/**
 * \file UdpPacketDistribution.hh
 * \brief Contains the template implementations for the \c NetworkKit::UdpPacketDistribution class.
 * \date 2019-08-15 11:39:20
 * \author Rolando J. Nieves
 */

#ifndef _FOUNDATION_NETWORKKIT_UDPPACKETDISTRIBUTION_CC_
#define _FOUNDATION_NETWORKKIT_UDPPACKETDISTRIBUTION_CC_

#include "UdpPacketDistribution.h"

namespace NetworkKit
{

template< typename NotificationCall >
unsigned long
UdpPacketDistribution::addNotificationCallback(NotificationCall &&callable)
{
    m_callableMap.emplace(
        CallableMap::value_type {
            NextCallbackId,
            std::forward< NotificationCall >(callable)
        }
    );

    return NextCallbackId++;
}

} // end namespace NetworkKit

#endif /* !_FOUNDATION_NETWORKKIT_UDPPACKETDISTRIBUTION_CC_ */

// vim: set ts=4 sw=4 expandtab:
