/**
 * \file      TcpServerInputSource.cpp
 * \brief     Definition of TcpServerConnectionListener class
 * \date      2012-12-14
 * \author    Ryan O'Farrell
 */

#include <algorithm>
#include <errno.h>

#include <CoreKit/Application.h>
#include <CoreKit/OsErrorException.h>
#include <CoreKit/PreconditionNotMetException.h>
#include <netinet/in.h>

#include "ConnectionCallbackT.h"
#include "TcpMessageCallbackT.h"
#include "TcpServerInputSource.h"

using std::bind1st;
using std::bind2nd;
using std::for_each;
using std::mem_fun;
using std::remove_if;
using std::string;

using CoreKit::AppLog;
using CoreKit::BoundMember;
using CoreKit::G_MyApp;

namespace NetworkKit
{

/**
 * \brief Deletes memory for single connection
 * \param aCallback allocated callback to delete
 */
static void deleteConnectionCallback(ConnectionCallback *aCallback)
{
    delete aCallback;
}

/**
 * \brief Deletes memory for single callback
 * \param aCallback allocated callback to delete
 */
static void deleteTcpMessageCallback(TcpMessageCallback *aCallback)
{
    delete aCallback;
}

TcpServerInputSource::TcpServerInputSource(CoreKit::RunLoop *i_loop, int i_portNumber,
                                           unsigned int maxClients)
    : m_loop(i_loop), m_maxClients(maxClients), m_serverSockFd(-1), m_portNumber(i_portNumber),
      clilen(0), m_listeners(), m_callbacks(), m_connectionCallbacks()
{
    this->createServerSocket();
}

TcpServerInputSource::TcpServerInputSource(CoreKit::RunLoop *i_loop, int i_portNumber,
                                           std::string i_serverIp, unsigned int maxClients)
    : m_loop(i_loop), m_maxClients(maxClients), m_serverSockFd(-1), m_portNumber(i_portNumber),
      clilen(0), m_listeners(), m_callbacks(), m_connectionCallbacks()
{
    struct in_addr address;
    if (!inet_aton(i_serverIp.c_str(), &address))
    {
        throw CoreKit::PreconditionNotMetException("Invalid server address");
    }
    this->createServerSocket(address.s_addr);
}

TcpServerInputSource::~TcpServerInputSource()
{
    for (auto listener : m_listeners)
    {
        // have to de-register input sources here
        // not exposed to user classes and must be de-registered before RunLoop is destroyed
        // Application Main Run Loop destroyed in Application destructor
        if (nullptr != m_loop && (-1 != listener->fileDescriptor()))
        {
            try
            {
                m_loop->deregisterInputSource(listener);
            }
            catch (CoreKit::OsErrorException &osError)
            {
                if (nullptr != G_MyApp)
                {
                    G_MyApp->log() << AppLog::LL_WARNING << "Error de-registering input source "
                                   << osError.what() << CoreKit::EndLog;
                }
            }
        }
        delete listener;
    }
    m_listeners.clear();

    // delete the callbacks registered with this class
    for_each(m_callbacks.begin(), m_callbacks.end(), &deleteTcpMessageCallback);
    m_callbacks.clear();

    for_each(m_connectionCallbacks.begin(), m_connectionCallbacks.end(), &deleteConnectionCallback);
    m_connectionCallbacks.clear();

    this->closeServerSocket();
}

void TcpServerInputSource::createServerSocket(in_addr_t serverAddress)
{
    // first, open socket
    m_serverSockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_serverSockFd < 0)
    {
        throw CoreKit::OsErrorException("Error opening socket", errno);
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = serverAddress;
    serv_addr.sin_port = htons(m_portNumber);

    // allow the socket to be immediately re-used after server stops
    int on = 1;

    setsockopt(m_serverSockFd, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(on));

    // next, bind socket
    if (bind(m_serverSockFd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::stringstream errMsg;
        errMsg << "Failed to bind socket to port : " << m_portNumber;
        throw CoreKit::OsErrorException(errMsg.str(), errno);
    }

    // listen for connections
    if (listen(m_serverSockFd, PENDING_QUEUE_LENGTH) < 0)
    {
        std::stringstream errMsg;
        errMsg << "Failed OS listen() call: " << m_portNumber;
        throw CoreKit::OsErrorException(errMsg.str(), errno);
    }
    clilen = sizeof(cli_addr);
}

void TcpServerInputSource::closeServerSocket()
{
    close(m_serverSockFd);
}

int TcpServerInputSource::fileDescriptor() const
{
    return m_serverSockFd;
}

static bool isListenerNull(TcpMessageInputSource *inputSource)
{
    return (NULL == inputSource);
}

void TcpServerInputSource::fireCallback()
{
    // waits until someone connects to the socket, then returns
    int sockFd = accept(m_serverSockFd, (struct sockaddr *)&cli_addr, &clilen);
    if (sockFd < 0)
    {
        std::stringstream errMsg;
        errMsg << "Accept failed, socket FD =  : " << m_serverSockFd
               << ". port num = " << m_portNumber;
        if (NULL != G_MyApp)
        {
            G_MyApp->log() << AppLog::LL_WARNING << errMsg.str() << CoreKit::EndLog;
        }
    }
    else
    {
        // check for any memory that is not currently used
        // can't use for_each because I want to set to NULL for next part
        for (unsigned int i = 0; i < m_listeners.size(); i++)
        {
            if (m_listeners.at(i)->fileDescriptor() <= 0)
            {
                delete m_listeners.at(i);
                m_listeners.at(i) = NULL;
            }
        }

        // remove any NULL listeners
        m_listeners.erase(remove_if(m_listeners.begin(), m_listeners.end(), isListenerNull),
                          m_listeners.end());

        if (m_listeners.size() < m_maxClients)
        {

            TcpMessageInputSource *listener =
                construct(TcpMessageInputSource::myType(), m_loop, sockFd);

            listener->addDisconnectionCallback(
                newConnectionCallback(BoundMember(this, &TcpServerInputSource::onDisconnection)));

            // register this class as a callback for messages
            listener->addTcpMessageCallback(
                newTcpMessageCallback(BoundMember(this, &TcpServerInputSource::onTcpMessage)));

            if (NULL != m_loop)
            {
                m_loop->registerInputSource(listener);
            }
            // create the new TcpMessageListener for this connection
            m_listeners.push_back(listener);

            if (NULL != G_MyApp)
            {
                G_MyApp->log() << AppLog::LL_DEBUG << "New connection accepted" << CoreKit::EndLog;
            }

            ConnectionNotification *notification =
                new ConnectionNotification(listener->getSocket(), ConnectionStates::CONNECTED);
            // notify all connection callbacks
            for_each(m_connectionCallbacks.begin(), m_connectionCallbacks.end(),
                     bind2nd(mem_fun(&ConnectionCallback::operator()), notification));

            delete notification;
        }
        else
        {
            if (NULL != G_MyApp)
            {
                G_MyApp->log() << AppLog::LL_ERROR
                               << "TcpServer: Could not accept new client connection, maximum "
                                  "number of clients ("
                               << m_maxClients << ") already connected" << CoreKit::EndLog;
            }
        }
    }
}

void TcpServerInputSource::addConnectionCallback(ConnectionCallback *theCallback)
{
    if (NULL == theCallback)
    {
        throw CoreKit::PreconditionNotMetException("Can not register NULL callback");
    }

    // add to this list
    m_connectionCallbacks.push_back(theCallback);
}

void TcpServerInputSource::addTcpMessageCallback(TcpMessageCallback *theCallback)
{
    if (NULL == theCallback)
    {
        throw CoreKit::PreconditionNotMetException("Can not register NULL callback");
    }
    // add to this list
    m_callbacks.push_back(theCallback);
}

void TcpServerInputSource::onTcpMessage(TcpMessageNotification *tcpMessageNotification)
{
    for_each(m_callbacks.begin(), m_callbacks.end(),
             bind2nd(mem_fun(&TcpMessageCallback::operator()), tcpMessageNotification));
}

void TcpServerInputSource::bufferData(size_t bufferSize)
{
    for_each(m_listeners.begin(), m_listeners.end(),
             bind2nd(mem_fun(&TcpMessageInputSource::bufferData), bufferSize));
}

void TcpServerInputSource::onDisconnection(ConnectionNotification *notification)
{
    for_each(m_connectionCallbacks.begin(), m_connectionCallbacks.end(),
             bind2nd(mem_fun(&ConnectionCallback::operator()), notification));
}

} /* namespace NetworkKit */
