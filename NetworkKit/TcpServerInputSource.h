/**
 * \file      TcpServerInputSource.h
 * \brief     Declaration of TcpServerConnectionListener class
 * \date      2012-12-14
 * \author    Ryan O'Farrell
 */

#ifndef TCPSERVERCONNECTIONLISTENER_H_
#define TCPSERVERCONNECTIONLISTENER_H_

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include <CoreKit/AppLog.h>
#include <CoreKit/Application.h>
#include <CoreKit/InputSource.h>
#include <CoreKit/RunLoop.h>

#include "ConnectionCallback.h"
#include "TcpMessageCallback.h"
#include "TcpMessageInputSource.h"

namespace NetworkKit
{

/**
 * \brief This class provides an Input Source for clients connecting to a TCP server port.
 * \details This is a single threaded class that is made for use with the \c CoreKit::RunLoop
 *  multiplexor.  A new InputSource is created for each connection to handle messages that arrive.
 * \author Ryan O'Farrell
 * \date 2012-12-14
 */
class TcpServerInputSource : public CoreKit::InputSource
{
    RF_CK_FACTORY_COMPATIBLE(TcpServerInputSource);

  public:
    /**
     * \brief Constructor
     * \param i_loop the \c RunLoop the Input Source is used in
     * \param i_portNumber the server port to use
     * \param i_maxClients maximum number of client connections
     * \throw CoreKit::OsErrorException if server socket fails to open
     */
    TcpServerInputSource(CoreKit::RunLoop *i_loop, int i_portNumber,
                         unsigned int i_maxClients = 10);

    /**
     * \brief Constructor
     * \param i_loop the \c RunLoop the Input Source is used in
     * \param i_portNumber the server port to use
     * \param i_serverIp the server IP to use
     * \param i_maxClients maximum number of client connections
     * \throw CoreKit::OsErrorException if server socket fails to open
     */
    TcpServerInputSource(CoreKit::RunLoop *i_loop, int i_portNumber, std::string i_serverIp,
                         unsigned int i_maxClients = 10);

    /**
     * \brief Destructor
     */
    virtual ~TcpServerInputSource();

    /**
     * \brief Creates and starts listening for clients
     */
    void createServerSocket(in_addr_t serverAddress = INADDR_ANY);

    /**
     * \brief Closes existing server socket
     */
    void closeServerSocket();

    /**
     * \brief Provides the socket FD for \c CoreKit::RunLoop multiplexor
     * \return socket FD
     */
    virtual int fileDescriptor() const;

    /**
     * \brief Called when a client connects to the server port to establish new connection
     * \throw CoreKit::OsErrorException if accept of new connection fails
     */
    virtual void fireCallback();

    /**
     * \brief Adds a new listener for messages that are received from any
     *        client that attaches to this server port.
     * \details This class takes ownership of the pointer parameter and will handles
     *  its deletion
     * \param theCallback function to handle new TCP messages
     * \throw CoreKit::PreconditionNotMetException if theCallback is NULL
     */
    virtual void addTcpMessageCallback(TcpMessageCallback *theCallback);

    /**
     * \brief Adds a new connection callback
     * \param theCallback callback to add
     * \throw CoreKit::PreconditionNotMetException if callback is NULL
     */
    virtual void addConnectionCallback(ConnectionCallback *theCallback);

    /**
     * \brief Causes all callbacks to be triggered only when bufferSize
     * bytes have been received.
     * \param bufferSize size of callback buffer
     */
    void bufferData(size_t bufferSize);

    /**
     * \brief Handler for TCP messages.
     * \details Forwards this message to any listeners connected to the server
     * \param tcpMessageNotification the message
     */
    void onTcpMessage(TcpMessageNotification *tcpMessageNotification);

    /**
     * \brief Notifies any liseteners that a client has disconnected
     * \param notification the connection notification
     */
    void onDisconnection(ConnectionNotification *notification);

  private:
    /**
     * \brief Private copy constructor.  Undefined to prevent copying.
     * \param other object to copy
     */
    TcpServerInputSource(const TcpServerInputSource &other);

    /**
     * \brief Private assignment operator.  Undefined to prevent copying.
     * \param other object to assign
     */
    TcpServerInputSource &operator=(const TcpServerInputSource &other);

    /** \c RunLoop reference */
    CoreKit::RunLoop *m_loop;
    /** Maximum number of client connections */
    unsigned int m_maxClients;
    /** Server socket */
    int m_serverSockFd;
    /** Server port */
    int m_portNumber;
    /** Socket size */
    socklen_t clilen;
    /** Server and client addresses */
    struct sockaddr_in serv_addr, cli_addr;
    /** Listeners for new messages on each connection */
    std::vector<TcpMessageInputSource *> m_listeners;
    /** Callbacks that are distributed across all listeners */
    std::vector<TcpMessageCallback *> m_callbacks;
    /** Callbacks for when a new client connects */
    std::vector<ConnectionCallback *> m_connectionCallbacks;
    /** Maximum pending connections */
    static const int PENDING_QUEUE_LENGTH = 5;
};

} /* namespace NetworkKit */
#endif /* TCPSERVERCONNECTIONLISTENER_H_ */
