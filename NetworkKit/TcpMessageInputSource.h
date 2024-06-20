/**
 * \file      TcpMessageInputSource.h
 * \brief     Declaration of TcpInputSource class
 * \date      2012-12-13
 * \author    Ryan O'Farrell
 */

#ifndef TCPINPUTSOURCE_H_
#define TCPINPUTSOURCE_H_

#include <string>
#include <vector>
#include <errno.h>
#include <stdint.h>
#include <sstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>

#include <CoreKit/InterruptListener.h>
#include <CoreKit/InputSource.h>
#include <CoreKit/PreconditionNotMetException.h>
#include <CoreKit/AppLog.h>
#include <CoreKit/RunLoop.h>
#include <CoreKit/factory.h>

#include "TcpSocket.h"
#include "ConnectionCallback.h"
#include "TcpMessageCallback.h"

namespace NetworkKit
{

/**
 * \brief Handles receiving messages from an open TCP socket
 * \details Messages will be distributed to all
 * registered callbacks when received.  On destruction, the socket will be closed.
 * \author Ryan O'Farrell
 * \date 2012-12-13 
 */
class TcpMessageInputSource: public CoreKit::InterruptListener,
        public CoreKit::InputSource
{
public:
RF_CK_FACTORY_COMPATIBLE(TcpMessageInputSource)
    ;

    /** \brief Defines the maximum message size */
    static const size_t READ_BUFFER_SIZE;

    /**
     * \brief Constructor
     * \param i_loop run loop to register input with
     */
    TcpMessageInputSource(CoreKit::RunLoop *i_loop);

    /**
     * \brief Constructor
     * \param i_loop run loop to register input with
     * \param i_sockFd the open socket to receive messages on this input
     */
    TcpMessageInputSource(CoreKit::RunLoop *i_loop, int i_sockFd);

    /**
     * Destructor
     */
    virtual ~TcpMessageInputSource();

    virtual int fileDescriptor() const;

    virtual CoreKit::InterruptListener* interruptListener() const;

    virtual void inputAvailableFrom(InputSource* theInputSource);

    virtual void fireCallback();

    /**
     * \brief Gets the socket instance for this input source
     * \param socket the socket to use for communication
     */
    virtual TcpSocket * getSocket();

    /**
     * \brief Sends data as bytes using TCP socket
     * \tparam VectorType the type of vector containing the data bytes
     * \param dataToSend the data as bytes
     * \return number of bytes sent on success, -1 on error
     */
    template<typename VectorType>
    int sendData(VectorType const& dataToSend) const
    {
         return m_socket->sendData(dataToSend);
    }

    /**
     * \brief Adds a callback for when a message is received on this socket
     * \param theCallback the object/function to call on message
     * \throw CoreKit::PreconditionNotMetException if callback is NULL
     */
    virtual void addTcpMessageCallback(TcpMessageCallback* theCallback);

    /**
     * \brief Adds a callback for when this socket becomes disconnected
     * \param theCallback the object/function to call on message
     * \throw CoreKit::PreconditionNotMetException if callback is NULL
     */
    virtual void addDisconnectionCallback(ConnectionCallback* theCallback);

    /**
     * \brief Causes all callbacks to be triggered only when bufferSize
     * bytes have been received.
     * \param bufferSize size of callback buffer
     */
    virtual void bufferData(size_t bufferSize);

private:

    /**
     * \brief Private copy constructor.  Undefined to prevent copying.
     * \param other object to copy
     */
    TcpMessageInputSource(const TcpMessageInputSource& other);

    /**
     * \brief Private assignment operator.  Undefined to prevent copying.
     * \param other object to assign
     */
    TcpMessageInputSource& operator=(const TcpMessageInputSource& other);
    /** Connected Socket */
    TcpSocket *m_socket;
    /** Callbacks */
    std::vector<TcpMessageCallback*> m_messageCallbacks;
    /** Callbacks */
    std::vector<ConnectionCallback*> m_disconnectionCallbacks;
    /** \c RunLoop reference */
    CoreKit::RunLoop *m_loop;
    /** Single notification used for all received messages */
    TcpMessageNotification *m_prototypeMessageNotification;
    /** Single notification used for all connection messages */
    ConnectionNotification *m_prototypeConnectionNotification;
    /** Used to trigger buffering of data for callbacks */
    bool m_buffering;

};

} /* namespace NetworkKit */
#endif /* TCPINPUTSOURCE_H_ */
