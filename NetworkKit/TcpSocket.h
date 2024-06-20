/**
 * \file TcpSocket.h
 * \brief Declaration of TcpSocket class
 * \date 2015-11-10
 * \author Ryan O'Farrell
 */

#ifndef TCPSOCKET_H_
#define TCPSOCKET_H_

//
// System Includes
//
#include <stdio.h>
#include <stdlib.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <string>
#include <vector>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>

//
// Library Includes
//
#include <CoreKit/CoreKit.h>

//
// Project Includes
//

namespace NetworkKit
{

/**
 * \brief Models a TCP socket connection
 * \author Ryan O'Farrell
 * \date 2015-11-10
 */
class TcpSocket
{
public:
    RF_CK_FACTORY_COMPATIBLE(TcpSocket)

    /**
     * \brief Constructor
     */
    TcpSocket();

    /**
     * \brief Destructor
     */
    virtual ~TcpSocket();

    /**
     * \brief Sends data as bytes using TCP socket
     * \tparam VectorType the type of vector containing the data bytes
     * \param dataToSend the data as bytes
     * \return number of bytes sent on success, -1 on error
     */
    template<typename VectorType>
    int sendData(VectorType const& dataToSend) const
    {
        if (-1 == m_sockFd)
        {
            return -1;
        }
        int retCode = 0;
        if (0 == dataToSend.size())
        {
            return retCode;
        }

        int n = send(m_sockFd, dataToSend.data(), dataToSend.size(),
                MSG_NOSIGNAL);
        retCode = n;

        if (n != dataToSend.size())
        {
            std::stringstream errMsg;
            errMsg << "Failed to write message to socket " << m_sockFd
                    << " Error: " << errno;
            if (NULL != CoreKit::G_MyApp)
            {
                CoreKit::G_MyApp->log() << CoreKit::AppLog::LL_WARNING << errMsg.str()
                        << CoreKit::EndLog;
            }
        }

        return retCode;
    }

    /**
     * \brief Sets a socket option and verifes that it has been set
     * \param level the level of the option
     * \param optName the name of the option
     * \param newSetting the new setting value for the option
     * \param description string description of the option for debug logging purposes
     * \return true if option set and read correctly, false otherwise
     */
    virtual bool setOption(int level, int optName, int newSetting, std::string description);

    /**
     * \brief Starts the connection process to the given server using a non-blocking connect call
     * \param serverAddress the address of the server to connect to
     * \return -1 if failure, 0 if immediately connected, 1 if connection started and pending
     */
    virtual int startConnection(struct sockaddr * serverAddress);

    /**
     * \brief Closes the socket FD if currently set
     * \details The user should determine when to close the socket.  Note that the \c NetworkKit::TcpMessageInputSource
     * maintains the same FD in some cases
     * \return 0 if closed succesfully, -1 if error
     */
    virtual int disconnect();

    /**
     * \brief Retrieves the current FD for this socket
     * \return socket FD
     */
    virtual int getSockFd() const;

    /**
     * \brief Sets the socket FD for this instance
     * \param sockFd the FD of this socket
     */
    virtual void setSockFd(int sockFd);

    /**
     * \brief Creates a new socket and assigns the resulting FD to the instance FD
     * \param blocking true if blocking connection, false if non-blocking
     * \return FD on success, -1 if socket call fails
     */
    virtual int createSocket(bool blocking);

    /**
     * \brief Sets this socket FD to blocking
     * \details Logs an error if there is a failure
     */
    virtual void setBlocking();

    /**
     * \brief Gets the current error code in the SO_ERROR socket option
     * \return current error code, or -1 if error occurs while reading
     */
    int getErrorCode() const;

private:
    int m_sockFd;

    /**
     * \brief Private copy constructor.  Undefined to prevent copying.
     * \param other object to copy
     */
    TcpSocket(const TcpSocket& other);

    /**
     * \brief Private assignment operator.  Undefined to prevent copying.
     * \param other object to assign
     */
    TcpSocket& operator=(const TcpSocket& other);
};

} /* namespace NetworkKit */
#endif /* TCPSOCKET_H_ */
