/**
 * \file      TcpClient.cpp
 * \brief     Definition of TcpClientInputSource class
 * \date      2012-12-13
 * \author    Ryan O'Farrell
 */

#include <CoreKit/OsErrorException.h>
#include <CoreKit/PreconditionNotMetException.h>
#include <CoreKit/AppLog.h>
#include <CoreKit/Application.h>
#include <CoreKit/RunLoop.h>

#include "TcpClient.h"
#include "TcpMessageCallbackT.h"
#include "ConnectionCallbackT.h"
#include "TcpMessageInputSource.h"

using std::for_each;
using std::string;
using std::bind2nd;
using std::bind1st;
using std::mem_fun;
using std::cerr;
using std::endl;
using CoreKit::OsErrorException;
using CoreKit::PreconditionNotMetException;
using CoreKit::AppLog;
using CoreKit::EndLog;
using CoreKit::G_MyApp;
using CoreKit::construct;
using CoreKit::destroy;

namespace NetworkKit
{

/**
 * \brief Deletes memory for single callback
 * \param aCallback allocated callback to delete
 */
static void deleteTcpMessageCallback(TcpMessageCallback *aCallback)
{
    delete aCallback;
}

/**
 * \brief Deletes memory for single callback
 * \param aCallback allocated callback to delete
 */
static void deleteConnectionCallback(ConnectionCallback *aCallback)
{
    delete aCallback;
}

TcpClient::TcpClient(int i_serverPortNum, std::string i_hostname,
        CoreKit::RunLoop *i_loop) :
        m_loop(i_loop), m_callbacks(), m_messageInputSource(
                new TcpMessageInputSource(m_loop)), m_serverPortNum(
                i_serverPortNum), m_hostname(i_hostname), m_connectionState(
                DISCONNECTED), m_disconnectionCallbacks(), m_keepCount(-1), m_keepInterval(
                -1), m_keepIdle(-1), m_timerFd(-1)
{
    //register disconnection callback for this class
    m_messageInputSource->addDisconnectionCallback(
            newConnectionCallback(
                    bind1st(mem_fun(&TcpClient::onDisconnection), this)));

    //register this class as a callback for messages
    m_messageInputSource->addTcpMessageCallback(
            newTcpMessageCallback(
                    bind1st(mem_fun(&TcpClient::onTcpMessage), this)));

    findHostIpAddress();
}

TcpClient::~TcpClient()
{
    removeMessageListener();
    deleteInputSource();
    deleteCallbacks();
}

void TcpClient::enableKeepAlive(int keepIdle, int keepInterval, int keepCount)
{
    m_keepCount = keepCount;
    m_keepInterval = keepInterval;
    m_keepIdle = keepIdle;
}

void TcpClient::timerExpired(int timerFd)
{
    struct timeval selectTimeout;
    selectTimeout.tv_sec = 0;
    selectTimeout.tv_usec = 0;
    fd_set outputFd;
    int selectResult = 0;

    FD_ZERO(&outputFd);
    FD_SET(m_messageInputSource->fileDescriptor(), &outputFd);
    selectResult = select(m_messageInputSource->fileDescriptor() + 1, NULL,
            &outputFd, NULL, &selectTimeout);
    if (selectResult > 0)
    {
        m_loop->deregisterTimer(m_timerFd);
        //set to disconnected as the default
        //this is overriden below if the connection succeeds
        m_connectionState = DISCONNECTED;
        if (FD_ISSET(m_messageInputSource->fileDescriptor(), &outputFd))
        {
            if (NULL != G_MyApp)
            {
                G_MyApp->log() << AppLog::LL_DEBUG
                        << "TcpClient : connect select returned, checking if connected."
                        << EndLog;
            }

            //check error in sock opt to see if connected
            int errorCode = m_messageInputSource->getSocket()->getErrorCode();
            if (errorCode != 0)
            {
                if (NULL != G_MyApp)
                {
                    G_MyApp->log() << AppLog::LL_WARNING
                            << "TcpClient : Client failed to connect : "
                            << strerror(errorCode) << "[" << errorCode << "]"
                            << EndLog;
                }
                m_messageInputSource->getSocket()->disconnect();
            }
            else
            {
                if (NULL != G_MyApp)
                {
                    G_MyApp->log() << AppLog::LL_DEBUG
                            << "TcpClient : Client connected" << EndLog;
                }
                if (NULL != m_loop)
                {
                    m_loop->registerInputSource(m_messageInputSource);
                }
                m_connectionState = CONNECTED;

                //set socket back to blocking
                m_messageInputSource->getSocket()->setBlocking();
            }
        }
    }
    else if (selectResult < 0)
    {
        m_loop->deregisterTimer(m_timerFd);
        m_connectionState = DISCONNECTED;
        if (NULL != G_MyApp)
        {
            G_MyApp->log() << AppLog::LL_WARNING
                    << "TcpClient : connect select failed with error : "
                    << strerror(errno) << EndLog;
        }
        m_messageInputSource->getSocket()->disconnect();
    }

}

int TcpClient::connect(bool blocking)
{
    int returnCode(-1);
    if (this->isConnected())
    {
        returnCode = 0;
    }
    else
    {

        //create the socket
        int socketReturn = m_messageInputSource->getSocket()->createSocket(
                blocking);
        if (socketReturn < 0)
        {
            if (NULL != G_MyApp)
            {
                G_MyApp->log() << AppLog::LL_WARNING
                        << "TcpClient : Create socket failed :  "
                        << strerror(errno) << "[" << errno << "]" << EndLog;
            }
            returnCode = -1;
        }
        else
        {
            //set keep alive options if needed
            if (m_keepCount > 0 && m_keepIdle > 0 && m_keepInterval > 0)
            {
                if (NULL != G_MyApp)
                {
                    G_MyApp->log() << AppLog::LL_INFO
                            << "Using TCP keep alive for client ("
                            << m_messageInputSource->fileDescriptor()
                            << ") , keep count = " << m_keepCount
                            << ", keep interval = " << m_keepInterval
                            << ", keep idle = " << m_keepIdle << EndLog;
                }
                if (m_messageInputSource->getSocket()->setOption(SOL_SOCKET,
                        SO_KEEPALIVE, 1, "SO_KEEPALIVE"))
                {
                    if (m_messageInputSource->getSocket()->setOption(
                            IPPROTO_TCP, TCP_KEEPCNT, m_keepCount,
                            "TCP_KEEPCNT"))
                    {
                        if (m_messageInputSource->getSocket()->setOption(
                                IPPROTO_TCP, TCP_KEEPINTVL, m_keepInterval,
                                "TCP_KEEPINTVL"))
                        {
                            //no need to check return, no action taken if false
                            m_messageInputSource->getSocket()->setOption(
                                    IPPROTO_TCP, TCP_KEEPIDLE, m_keepIdle,
                                    "TCP_KEEPIDLE");
                        }
                    }
                }
            }
            else
            {
                if (NULL != G_MyApp)
                {
                    G_MyApp->log() << AppLog::LL_INFO
                            << "Not using TCP keep alive for client ("
                            << m_messageInputSource->fileDescriptor() << EndLog;
                }

            }

            //attempt to start connection
            int connectionCode =
                    m_messageInputSource->getSocket()->startConnection(
                            (struct sockaddr *) &m_serverAddress);

            returnCode = connectionCode;

            switch (connectionCode)
            {
            case -1:
                //failure
                m_connectionState = DISCONNECTED;
                break;
            case 0:
                //connected immediately
                if (NULL != m_loop)
                {
                    m_loop->registerInputSource(m_messageInputSource);
                }
                m_connectionState = CONNECTED;
                break;
            case 1:
                //pending
                m_connectionState = PENDING;
                if (NULL != m_loop)
                {
                    m_timerFd = m_loop->registerTimerWithInterval(0.1, this,
                            true);
                }
                break;
            default:
                if (NULL != G_MyApp)
                {
                    G_MyApp->log() << AppLog::LL_WARNING << 
                    "TcpClient connect: Unknown connection state " << 
                    connectionCode << EndLog;
                }
                break;
            }
        }
    }

    return returnCode;
}

void TcpClient::disconnect()
{
    if (m_connectionState == CONNECTED)
    {
        this->removeMessageListener();
    }

    m_connectionState = DISCONNECTED;
}

bool TcpClient::isConnected() const
{
    return (m_connectionState == CONNECTED);
}

bool TcpClient::isPending() const
{
    return (m_connectionState == PENDING);
}

void TcpClient::onDisconnection(ConnectionNotification *notification)
{
//notify callbacks that this socket is disconnected
    for_each(m_disconnectionCallbacks.begin(), m_disconnectionCallbacks.end(),
            bind2nd(mem_fun(&ConnectionCallback::operator()), notification));

    m_connectionState = DISCONNECTED;
}

void TcpClient::findHostIpAddress()
{
    bzero((char*) (&m_serverAddress), sizeof(m_serverAddress));
    m_serverAddress.sin_family = AF_INET;
//first, try to use input as IP address
    int conversionResult = inet_aton(m_hostname.c_str(),
            &m_serverAddress.sin_addr);
//only if the conversion failed, try to use it as a string hostname
    if (!conversionResult)
    {
        struct hostent *server;
        //if that fails, try to use it as host name
        server = gethostbyname(m_hostname.c_str());
        if (server == NULL)
        {
            std::stringstream errMsg;
            errMsg << "No such host " << m_hostname;
            throw OsErrorException(errMsg.str(), errno);
        }
        else
        {
            bcopy((char *) server->h_addr,
            (char *)&m_serverAddress.sin_addr.s_addr,
            server->h_length);
        }
    }

    m_serverAddress.sin_port = htons(m_serverPortNum);
}

void TcpClient::addTcpMessageCallback(TcpMessageCallback* theCallback)
{
    if (NULL == theCallback)
    {
        throw PreconditionNotMetException("Can not register NULL callback");
    }

    m_callbacks.push_back(theCallback);
}

void TcpClient::onTcpMessage(TcpMessageNotification *tcpMessageNotification)
{
    for_each(m_callbacks.begin(), m_callbacks.end(),
            bind2nd(mem_fun(&TcpMessageCallback::operator()),
                    tcpMessageNotification));
}

void TcpClient::addDisconnectionCallback(ConnectionCallback* theCallback)
{
    if (NULL == theCallback)
    {
        throw PreconditionNotMetException("Can not register NULL callback");
    }

    m_disconnectionCallbacks.push_back(theCallback);
}

void TcpClient::removeMessageListener()
{
    if (NULL != m_loop)
    {
        try
        {
            if (m_messageInputSource->fileDescriptor() > 0)
            {
                m_loop->deregisterInputSource(m_messageInputSource);
            }
            m_messageInputSource->getSocket()->disconnect();
        }
        catch (CoreKit::OsErrorException &osError)
        {
            if (NULL != G_MyApp)
            {
                G_MyApp->log() << AppLog::LL_DEBUG
                        << "Error de-registering input source "
                        << osError.what() << CoreKit::EndLog;
            }
        }
    }
}

void TcpClient::deleteInputSource()
{
    delete m_messageInputSource;
    m_messageInputSource = NULL;
}

void TcpClient::deleteCallbacks()
{
    for_each(m_callbacks.begin(), m_callbacks.end(), &deleteTcpMessageCallback);
    m_callbacks.clear();

    for_each(m_disconnectionCallbacks.begin(), m_disconnectionCallbacks.end(),
            &deleteConnectionCallback);
    m_disconnectionCallbacks.clear();
}

void TcpClient::bufferData(size_t bufferSize)
{
// Created in this class, so no need for NULL check
    m_messageInputSource->bufferData(bufferSize);
}

} /* namespace NetworkKit */
