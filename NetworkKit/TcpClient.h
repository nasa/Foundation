/**
 * \file      TcpClient.h
 * \brief     Declaration of TcpClientInputSource class
 * \date      2012-12-13
 * \author    Ryan O'Farrell
 */

#ifndef TCPCLIENT_H_
#define TCPCLIENT_H_

#include <CoreKit/PreconditionNotMetException.h>
#include <CoreKit/AppLog.h>

#include "TcpMessageInputSource.h"
#include "TcpSocket.h"

namespace NetworkKit
{

/**
 * \brief Connects to a TCP server port and allows sending and receiving messages.
 * \author Ryan O'Farrell
 * \date 2012-12-13
 */
class TcpClient : public CoreKit::InterruptListener
{
RF_CK_FACTORY_COMPATIBLE(TcpClient)
    ;
public:

    /**
     * \brief Constructor
     * \param i_serverPortNum the server port to connect to
     * \param i_hostname the server host name or IP address
     * \param i_loop the \c RunLoop processing this connection
     */
    TcpClient(int i_serverPortNum, std::string i_hostname,
            CoreKit::RunLoop *i_loop);

    /**
     * \brief The destructor
     */
    virtual ~TcpClient();

    /**
     * \brief Enables TCP Keep Alive probes.  Must be called prior to \c connect
     * \param keepIdle time connection is idle before first probe (default 600 seconds)
     * \param keepInterval time between probes (default 300 seconds)
     * \param keepCount number of probes before giving up (default 10)
     */
    virtual void enableKeepAlive(int keepIdle = 600, int keepInterval = 300,
            int keepCount = 10);

    /**
     * \brief Connects this TCP socket to the server.
     * \details Must be called before sending a message
     * \param blocking true for blocking connection, false for non-blocking
     * \return -1 if failure, 0 if immediately connected, 1 if connection started and pending
     */
    virtual int connect(bool blocking = true);

    /**
     * \brief Disconnects this TCP socket to the server.
     */
    virtual void disconnect();

    /**
     * \brief Tells whether this client is currently connected to the server.
     * \return true if connected, false if pending or disconnceted
     */
    bool isConnected() const;

    /**
     * \brief Tells whether this client is currently waiting on a connection to complete
     * \return true if connection is pending, false otherwise
     */
    bool isPending() const;

    /**
     * \brief Adds a listener for messages received on the socket connection
     * \details This class takes ownership of the pointer parameter and will handle
     *  its deletion
     * \param theCallback the function to call for messages
     * \throw CoreKit::PreconditionNotMetException if theCallback is NULL
     */
    void addTcpMessageCallback(TcpMessageCallback* theCallback);

    /**
     * \brief Adds a callback for when this socket becomes disconnected
     * \details This class will take ownership of the callback pointer and delete all callbacks
     * upon instance destruction
     * \param theCallback the object/function to call on message
     * \throw CoreKit::PreconditionNotMetException if callback is NULL
     */
    void addDisconnectionCallback(ConnectionCallback* theCallback);

    /**
     * \brief Callback for when the socket for this client is disconnected.
     * \details When this method is called, this class will in turn notify any registered callbacks
     * and set its internal state to disconnected
     * \param notification the Connection Notification object
     */
    virtual void onDisconnection(ConnectionNotification *notification);

    /**
     * \brief Handler for TCP messages
     * \details Forwards this message to any listeners connected to the server
     * \param tcpMessageNotification the message
     */
    virtual void onTcpMessage(TcpMessageNotification *tcpMessageNotification);

    /**
     * \brief Sends data over socket as vector of bytes.
     * \pre socket is connected
     * \tparam VectorType the type of vector containing the data bytes
     * \param dataToSend the data
     * \return 0 on success, -1 on error
     */
    template<typename VectorType>
    int sendData(VectorType const& dataToSend) const
    {
        if (!this->isConnected())
        {
            return -1;
        }
        // Created in this class, so no need for NULL check
        return m_messageInputSource->sendData(dataToSend);
    }

    /**
     * \brief Causes all callbacks to be triggered only when bufferSize bytes have been received.
     * \param bufferSize size of callback buffer
     */
    void bufferData(size_t bufferSize);

    /**
     * \brief Timer for handling connection monitoring
     * \param timerFd the timer that expired
     */
    virtual void timerExpired(int timerFd);

private:

    /**
     * \brief Private copy constructor.  Undefined to prevent copying.
     * \param other object to copy
     */
    TcpClient(const TcpClient& other);

    /**
     * \brief Private assignment operator.  Undefined to prevent copying.
     * \param other object to assign
     */
    TcpClient& operator=(const TcpClient& other);

    void removeMessageListener();
    void deleteInputSource();
    void deleteCallbacks();
    void findHostIpAddress();

    enum ConnectionState
    {
        DISCONNECTED,
        PENDING,
        CONNECTED
    };

    /** \c RunLoop reference */
    CoreKit::RunLoop *m_loop;
    /** Callbacks - keep track of these so that we can delete them */
    std::vector<TcpMessageCallback *> m_callbacks;
    /** Input source for receiving messages */
    NetworkKit::TcpMessageInputSource *m_messageInputSource;
    /** Server port to connect to */
    int m_serverPortNum;
    /** Server host to connect to */
    std::string m_hostname;
    /** whether the socket is connected */
    ConnectionState m_connectionState;
    /** server address */
    struct sockaddr_in m_serverAddress;
    /** Callbacks */
    std::vector<ConnectionCallback*> m_disconnectionCallbacks;
    /** Time before first keep alive probe */
    int m_keepCount;
    /** Time between keep alive probes */
    int m_keepInterval;
    /** Number of probes before closed */
    int m_keepIdle;
    /** Timer FD for connect select */
    int m_timerFd;
};

} /* namespace NetworkKit */
#endif /* TCPCLIENTINPUTSOURCE_H_ */
