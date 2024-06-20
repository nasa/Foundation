/**
 * \file      TcpMessageInputSource.cpp
 * \brief     Definition of TcpInputSource class
 * \date      2012-12-13
 * \author    Ryan O'Farrell
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <iostream>
#include <algorithm>
#include <iterator>

#include <CoreKit/PreconditionNotMetException.h>
#include <CoreKit/OsErrorException.h>
#include <CoreKit/AppLog.h>
#include <CoreKit/Application.h>
#include <CoreKit/RunLoop.h>

#include "TcpMessageInputSource.h"
#include "TcpMessageCallback.h"
#include "TcpMessageCallback.h"


using std::for_each;
using std::string;
using std::bind2nd;
using std::mem_fun;
using std::vector;

using CoreKit::AppLog;
using CoreKit::G_MyApp;
using CoreKit::PreconditionNotMetException;
using CoreKit::construct;
using CoreKit::destroy;

namespace NetworkKit
{

const size_t TcpMessageInputSource::READ_BUFFER_SIZE = 2048;

TcpMessageInputSource::TcpMessageInputSource(CoreKit::RunLoop *i_loop) :
        m_socket(NULL), m_messageCallbacks(), m_disconnectionCallbacks(), m_loop(
                i_loop), m_prototypeMessageNotification(NULL), m_prototypeConnectionNotification(
                NULL), m_buffering(false)
{
    m_socket = construct(TcpSocket::myType());
}

TcpMessageInputSource::TcpMessageInputSource(CoreKit::RunLoop *i_loop,
        int i_sockFd) :
        m_socket(NULL), m_messageCallbacks(), m_disconnectionCallbacks(), m_loop(
                i_loop), m_prototypeMessageNotification(NULL), m_prototypeConnectionNotification(
                NULL), m_buffering(false)
{
    m_socket = construct(TcpSocket::myType());
    m_socket->setSockFd(i_sockFd);
}

/**
 * \brief Deletes memory for single callback
 * \param aCallback allocated callback to delete
 */
static void deleteTcpMessageCallback(TcpMessageCallback *aCallback)
{
    delete aCallback;
}

/**
 * \brief Deletes memory for single connection
 * \param aCallback allocated callback to delete
 */
static void deleteConnectionCallback(ConnectionCallback *aCallback)
{
    delete aCallback;
}

TcpMessageInputSource::~TcpMessageInputSource()
{
    //close socket to make sure no new messages arrive
    if (NULL != m_socket)
    {
        m_socket->disconnect();
    }

    delete m_prototypeMessageNotification;
    m_prototypeMessageNotification = NULL;

    delete m_prototypeConnectionNotification;
    m_prototypeConnectionNotification = NULL;

    //delete the callbacks registered with this class
    for_each(m_messageCallbacks.begin(), m_messageCallbacks.end(),
            &deleteTcpMessageCallback);
    m_messageCallbacks.clear();

    for_each(m_disconnectionCallbacks.begin(), m_disconnectionCallbacks.end(),
            &deleteConnectionCallback);
    m_disconnectionCallbacks.clear();

    delete m_socket;
    m_socket = NULL;
}

int TcpMessageInputSource::fileDescriptor() const
{
    return m_socket->getSockFd();
}

CoreKit::InterruptListener* TcpMessageInputSource::interruptListener() const
{
    return const_cast<TcpMessageInputSource*>(this);
}

void TcpMessageInputSource::fireCallback()
{
    this->inputAvailableFrom(this);
}

void TcpMessageInputSource::inputAvailableFrom(InputSource *inputSource)
{
    /* this can't be done in the constructor because the TcpMessageNotification
     * needs a pointer to "this" object
     */
    if (NULL == m_prototypeMessageNotification)
    {
        bufferData(0);
    }

    size_t originalSize = m_prototypeMessageNotification->m_message.size();
    size_t bufferSize = m_prototypeMessageNotification->m_message.capacity()
            - originalSize;

    ssize_t recvResult = 0;

    /* The system call recv provides a flag to wait until all bytes are received before returning. (MSG_WAITALL)
     * This is not compatible with the CoreKit::RunLoop class.  This callback executes within that loop, and
     * a blocking call here will block all execution of the application.
     */
    m_prototypeMessageNotification->m_message.resize(originalSize + bufferSize);

    recvResult = read(m_socket->getSockFd(),
            (void*) (m_prototypeMessageNotification->m_message.data()
                    + originalSize), bufferSize);

    if (0 > recvResult)
    {
        //error reading socket
        if (NULL != G_MyApp)
        {
            G_MyApp->log() << CoreKit::AppLog::LL_ERROR
                    << "Error reading from socket number = "
                    << m_socket->getSockFd() << "\n" << "Error number = "
                    << errno << CoreKit::EndLog;
        }
    }
    else if (0 == recvResult)
    {

        //Socket is closed, de-register from input to avoid invalid functionality
        if (NULL != G_MyApp)
        {
            G_MyApp->log() << CoreKit::AppLog::LL_WARNING << "Socket closed:  "
                    << m_socket->getSockFd() << CoreKit::EndLog;
        }

        if (NULL != m_loop)
        {
            try
            {
                m_loop->deregisterInputSource(this);
            }
            catch (CoreKit::OsErrorException &osError)
            {
                if (NULL != G_MyApp)
                {
                    G_MyApp->log() << AppLog::LL_WARNING
                            << "Error de-registering input source "
                            << osError.what() << CoreKit::EndLog;
                }
            }
        }

        //notify callbacks that this socket is disconnected
        if (NULL == m_prototypeConnectionNotification)
        {
            m_prototypeConnectionNotification = new ConnectionNotification(m_socket,
                    ConnectionStates::DISCONNECTED);
        }

        for_each(m_disconnectionCallbacks.begin(),
                m_disconnectionCallbacks.end(),
                bind2nd(mem_fun(&ConnectionCallback::operator()),
                        m_prototypeConnectionNotification));

        int closeReturn = m_socket->disconnect();
        if (closeReturn < 0)
        {
            G_MyApp->log() << AppLog::LL_WARNING
                    << "Error closing socket, error =  " << closeReturn
                    << CoreKit::EndLog;
        }

        //clear the message notification in case this socket is later re-connected
        m_prototypeMessageNotification->m_message.clear();
    }
    else
    {
        m_prototypeMessageNotification->m_message.resize(
                originalSize + recvResult);

        if (!m_buffering
                || m_prototypeMessageNotification->m_message.size()
                        == m_prototypeMessageNotification->m_message.capacity())
        {
            clock_gettime(CLOCK_REALTIME,
                    &(m_prototypeMessageNotification->m_acqTime));
            for_each(m_messageCallbacks.begin(), m_messageCallbacks.end(),
                    bind2nd(mem_fun(&TcpMessageCallback::operator()),
                            m_prototypeMessageNotification));

            m_prototypeMessageNotification->m_message.clear();
        }
    }
}

void TcpMessageInputSource::addTcpMessageCallback(
        TcpMessageCallback* theCallback)
{
    if (NULL == theCallback)
    {
        throw CoreKit::PreconditionNotMetException("Can not add NULL callback");
    }
    m_messageCallbacks.push_back(theCallback);
}

void TcpMessageInputSource::addDisconnectionCallback(
        ConnectionCallback* theCallback)
{
    if (NULL == theCallback)
    {
        throw CoreKit::PreconditionNotMetException("Can not add NULL callback");
    }
    m_disconnectionCallbacks.push_back(theCallback);
}

void TcpMessageInputSource::bufferData(size_t bufferSize)
{
    TcpMessageNotification *newNotif = NULL;

    if (0 == bufferSize)
    {
        m_buffering = false;
        bufferSize = READ_BUFFER_SIZE;
    }
    else
    {
        m_buffering = true;
    }

    if (!m_prototypeMessageNotification
            || bufferSize
                    != m_prototypeMessageNotification->m_message.capacity())
    {
        newNotif = new TcpMessageNotification(bufferSize, m_socket);
        delete m_prototypeMessageNotification;
        m_prototypeMessageNotification = newNotif;
    }
}

TcpSocket * TcpMessageInputSource::getSocket()
{
    return m_socket;
}

} /* namespace NetworkKit */
