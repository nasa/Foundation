/**
 * \file UdpPacketNotification.h
 * \brief Contains the definition of the \c NetworkKit::UdpPacketNotification family of classes.
 * \date 2019-08-14 18:40:15
 * \author Rolando J. Nieves
 */

#ifndef _FOUNDATION_NETWORKKIT_UDPPACKETNOTIFICATION_H_
#define _FOUNDATION_NETWORKKIT_UDPPACKETNOTIFICATION_H_

#include <cstdlib>
#include <string>

#include <CoreKit/CoreKit.h>

namespace NetworkKit
{

/**
 * \brief Base class to model a recieved Unix or IP UDP packet
 * \author Rolando J. Nieves
 * \date 2019-08-14
 */
class UdpPacketNotification
{
public:
    /** \brief The maximum supported packet size */
    static constexpr std::size_t MAX_PACKET_SIZE = (64u * 1024u);
    /** \brief The time when the UDP packet was received */
    double acqTime;
    /** \brief address type */
    int addressFamily;
    /** \brief Received packet contents */
    CoreKit::FixedByteVector packetContents;

    /** 
     * \brief Consturctor
     */
    UdpPacketNotification();

    /** 
     * \brief Constructor
     * \tparam ByteVector the type of vector containing packet contents
     * \param in_acqTime the acquisition time of the received packet
     * \param in_addressFamily the address family of the received packet
     * \param in_packetContents the received packet contents
     */
    template< typename ByteVector >
    UdpPacketNotification(
        double in_acqTime,
        int in_addressFamily,
        ByteVector const& in_packetContents
    );

    /**
     * \brief Copy constructor
     * \param other instance to copy
     */
    UdpPacketNotification(UdpPacketNotification const& other);

    UdpPacketNotification(UdpPacketNotification&& other) = delete;

    /**
     * \brief Destructor
     */
    virtual ~UdpPacketNotification() = default;

    /**
     * \brief Determine if the packet is valid
     * \return true if valid, false if invalid
     */
    virtual bool isValid() const;

    /**
     * \brief Copy assignment operator
     * \param other instance to copy
     * \return updated instance
     */
    UdpPacketNotification& operator=(UdpPacketNotification const& other);

    UdpPacketNotification& operator=(UdpPacketNotification&& other) = delete;
};


/**
 * \brief Modles a UNIX socket UDP packet
 * \author Rolando J. Nieves
 * \date 2019-08-14
 */
class UdpUxPacketNotification : public UdpPacketNotification
{
public:
    /** \brief the UNIX socket path */
    std::string socketPath;

    /**
     * \brief Constructor
     */
    UdpUxPacketNotification();

    /** 
     * \brief Constructor
     * \tparam ByteVector the type of vector containing packet contents
     * \param in_acqTime the acquisition time of the received packet
     * \param m_socketPath the UNIX socket path
     * \param in_packetContents the received packet contents
     */
    template< typename ByteVector >
    UdpUxPacketNotification(
        double in_acqTime,
        std::string const& m_socketPath,
        ByteVector const& in_packetContents
    );

    /**
     * \brief Copy constructor
     * \param other instance to copy
     */
    UdpUxPacketNotification(UdpUxPacketNotification const& other);

    UdpUxPacketNotification(UdpUxPacketNotification&& other) = delete;

    /**
     * \brief Destructor
     */
    virtual ~UdpUxPacketNotification() = default;

    virtual bool isValid() const override;

    /**
     * \brief Copy assignment operator
     * \param other instance to copy
     * \return updated instance
     */
    UdpUxPacketNotification& operator=(UdpUxPacketNotification const& other);

    UdpUxPacketNotification& operator=(UdpUxPacketNotification&& other) = delete;
};


/**
 * \brief Models a UDP/IP packet
 * \author Rolando J. Nieves
 * \date 2019-08-14
 */
class UdpIpPacketNotification : public UdpPacketNotification
{
public:
    /** \brief the sender IP address */
    std::string ipAddress;
    /** \brief the sender port */
    int port;


    /**
     * \brief Constructor
     */
    UdpIpPacketNotification();

    /** 
     * \brief Constructor
     * \tparam ByteVector the type of vector containing packet contents
     * \param in_acqTime the acquisition time of the received packet
     * \param in_ipAddress the IP address of the sender
     * \param in_port the IP port of the sender
     * \param in_packetContents the received packet contents
     */
    template< typename ByteVector >
    UdpIpPacketNotification(
        double in_acqTime,
        std::string const& in_ipAddress,
        int in_port,
        ByteVector const& in_packetContents
    );

    /**
     * \brief Copy constructor
     * \param other instance to copy
     */
    UdpIpPacketNotification(UdpIpPacketNotification const& other);

    UdpIpPacketNotification(UdpIpPacketNotification&& other) = delete;

    /**
     * \brief Destructor
     */
    virtual ~UdpIpPacketNotification() = default;

    virtual bool isValid() const override;

    /**
     * \brief Copy assignment operator
     * \param other instance to copy
     * \return updated instance
     */
    UdpIpPacketNotification& operator=(UdpIpPacketNotification const& other);

    UdpIpPacketNotification& operator=(UdpIpPacketNotification&& other) = delete;
};

} // end namespace NetworkKit

#endif /* !_FOUNDATION_NETWORKKIT_UDPPACKETNOTIFICATION_H_ */

// vim: set ts=4 sw=4 expandtab:
