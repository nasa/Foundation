/**
 * \file UdpPacketDistribution.h
 * \brief Contains the definition of the \c NetworkKit::UdpPacketDistribution class.
 * \date 2019-08-14 18:39:10
 * \author Rolando J. Nieves
 */

#ifndef _FOUNDATION_NETWORKKIT_UDPPACKETDISTRIBUTION_H_
#define _FOUNDATION_NETWORKKIT_UDPPACKETDISTRIBUTION_H_

#include <functional>
#include <unordered_map>
#include <atomic>
#include <memory>

#include <CoreKit/CoreKit.h>

#include <NetworkKit/UdpPacketNotification.hh>


namespace NetworkKit
{

class UdpSocket;

/**
 * \brief Distributes received UDP packets to registered callbacks
 * \author Rolando J. Nieves
 * \date 2019-08-04
 */
class UdpPacketDistribution : public CoreKit::InterruptListener
{
    RF_CK_FACTORY_COMPATIBLE(UdpPacketDistribution)
private:
    using NotificationCallable = std::function< void (UdpPacketNotification const&) >;
    using CallableMap = std::unordered_map< unsigned long, NotificationCallable >;
    using NotificationPtr = std::unique_ptr< UdpPacketNotification >;

    static std::atomic_ulong NextCallbackId;

    CallableMap m_callableMap;
    NotificationPtr m_notificationObj;

    void readInetPacket(UdpSocket *theSocket);

    void readUxPacket(UdpSocket *theSocket);

public:
    /**
     * \brief Constructor
     */
    UdpPacketDistribution() = default;

    /**
     * \brief Destructor
     */
    virtual ~UdpPacketDistribution();

    /**
     * \brief Registers a notification callback for UDP packets 
     * \tparam NotificationCall the type of the callback
     * \param callable the callback to register
     * \return the ID of the registered callback
     */
    template <typename NotificationCall>
    unsigned long addNotificationCallback(NotificationCall &&callable);

    /**
     * \brief Removes the callback that matches the provided ID
     * \param callableId the ID to remove
     */
    void removeNotificationCallback(unsigned long callableId);

    /**
     * \brief Handle input of a new UDP packet
     * \param source the input source that generated data
     */
    virtual void inputAvailableFrom(CoreKit::InputSource *source) override;

    // Copy and move not allowed
    UdpPacketDistribution(UdpPacketDistribution const& other) = delete;
    UdpPacketDistribution(UdpPacketDistribution&& other) = delete;
    UdpPacketDistribution& operator=(UdpPacketDistribution const& other) = delete;
    UdpPacketDistribution& operator=(UdpPacketDistribution&& other) = delete;
};

} // end namespace NetworkKit

#endif /* !_FOUNDATION_NETWORKKIT_UDPPACKETDISTRIBUTION_H_ */

// vim: set ts=4 sw=4 expandtab:
