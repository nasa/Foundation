/**
 * \file UdpSocket.h
 * \brief Contains the definition of the \c NetworkKit::UdpSocket class.
 * \date 2019-08-14 18:38:02
 * \author Rolando J. Nieves
 */

#ifndef _FOUNDATION_NETWORKKIT_UDPSOCKET_H_
#define _FOUNDATION_NETWORKKIT_UDPSOCKET_H_

#include <cstdlib>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include <CoreKit/CoreKit.h>

namespace NetworkKit
{

/**
 * \brief Models a TCP socket connection
 * \author Rolando J. Nieves
 * \date 2019-08-14
 */
class UdpSocket : public CoreKit::InputSource
{
    RF_CK_FACTORY_COMPATIBLE(UdpSocket)
private:
    struct sockaddr_in m_sockaddrIp;
    struct sockaddr_un m_sockaddrUn;
    int m_selectedFamily;
    int m_socketFd;
    CoreKit::InterruptListener *m_listener;
    
public:

    /**
     * \brief Constructor for a UNIX socket
     * \param uxPath the UNIX socket path
     */
    explicit UdpSocket(std::string const &uxPath);

    /**
     * \brief Constructor for a UDP/IP socket
     * \param ipAddress the IP address to bind
     * \param port the port to bind
     */
    UdpSocket(std::string const &ipAddress, int port);

    /**
     * \brief Destructor
     */
    virtual ~UdpSocket();

    /**
     * \brief Binds the UDP socket to the constructed address
     * \param listener the listener instance for handling received UDP packets
     */
    virtual void initialize(CoreKit::InterruptListener *listener);

    /**
     * \brief Stops this socket from receiving or sending any future data
     */
    virtual void terminate();

    /**
     * \brief Determine the selected family for this socket
     * \return AF_UNIX or AF_INET
     */
    inline int selectedFamily() const
    { return m_selectedFamily; }

    /**
     * \brief Get the UNIX socket path
     * \return UNIX socket path for UNIX socket, empty string for INET socket
     */
    std::string uxPath() const;

    /**
     * \brief Get the IP address
     * \return IP address for INET socket, empty string for UNIX socket
     */
    std::string ipAddress() const;

    /**
     * \brief Get the socket port
     * \return port number for INET socket, -1 for UNIX socket
     */
    int port() const;
    
    /**
     * \brief Send data to a UNIX socket
     * \tparam ByteVector the type of vector containing the data bytes
     * \param uxPath the destination UNIX socket path
     * \param packetContents the packet contents to send
     *
     * \return Number of bytes sent, or \c -1 if an error occurs.
     */
    template< typename ByteVector >
    ssize_t sendTo(std::string const& uxPath, ByteVector const& packetContents);

    /**
     * \brief Send data to UDP/IP socket
     * \tparam ByteVector the type of vector containing the data bytes
     * \param ipAddr the destination IP address
     * \param port the destination port
     * \param packetContents the packet contents to send
     *
     * \return Number of bytes sent, or \c -1 if an error occurs.
     */
    template< typename ByteVector >
    ssize_t sendTo(std::string const& ipAddr, int port, ByteVector const& packetContents);

    /**
     * \brief Read the latest data received by the UNIX socket
     * \details non-blocking call
     * \tparam ByteVector the type of vector containing the data bytes
     * \param[out] uxPath output parameter for the source UNIX socket path of received data
     * \param[out] packetContents output parameter for received data
     *
     * \return Number of bytes received in packet.
     */
    template< typename ByteVector >
    std::size_t receiveFrom(std::string& uxPath, ByteVector& packetContents);

    /**
     * \brief Read the latest data received by the UDP/IP socket
     * \details non-blocking call
     * \tparam ByteVector the type of vector containing the data bytes
     * \param[out] ipAddr output parameter for the source IP address of received data
     * \param[out] port output parameter for the source port of received data
     * \param[out] packetContents output parameter for received data
     *
     * \return Number of bytes received in packet.
     */
    template< typename ByteVector >
    std::size_t receiveFrom(std::string& ipAddr, int& port, ByteVector& packetContents);

    virtual int fileDescriptor() const override;

    virtual CoreKit::InterruptListener* interruptListener() const override;

    virtual void fireCallback() override;

    // Copy and move not allowed
    UdpSocket(UdpSocket const& other) = delete;
    UdpSocket(UdpSocket&& other) = delete;
    UdpSocket& operator=(UdpSocket const& other) = delete;
    UdpSocket& operator=(UdpSocket&& other) = delete;
};

} // end namespace NetworkKit

#endif /* !_FOUNDATION_NETWORKKIT_UDPSOCKET_H_ */

// vim: set ts=4 sw=4 expandtab:
