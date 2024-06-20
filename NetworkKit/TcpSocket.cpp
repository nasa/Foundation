/**
 * \file TcpSocket.cpp
 * \brief Definition of TcpSocket class
 * \date 2015-11-10
 * \author Ryan O'Farrell
 */

#include "TcpSocket.h"

using CoreKit::OsErrorException;
using CoreKit::G_MyApp;
using CoreKit::AppLog;
using CoreKit::EndLog;

namespace NetworkKit
{

TcpSocket::TcpSocket() :
        m_sockFd(-1)
{

}

TcpSocket::~TcpSocket()
{
}

int TcpSocket::startConnection(struct sockaddr * serverAddress)
{
    int returnCode(-1);
    int result = ::connect(m_sockFd, serverAddress, sizeof(struct sockaddr));
    if (result < 0)
    {
        if (EINPROGRESS != errno)
        {
            //connect failed
            this->disconnect();
            returnCode = -1;
        }
        else
        {
            //connect pending
            returnCode = 1;
        }
    }
    else
    {
        //connect completed
        returnCode = 0;
    }

    return returnCode;
}

int TcpSocket::createSocket(bool blocking)
{
    if (blocking)
    {
        m_sockFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    }
    else
    {
        m_sockFd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
    }
    return m_sockFd;
}

int TcpSocket::disconnect()
{
    int closed(0);
    if (m_sockFd > 0)
    {
        closed = close(m_sockFd);
        m_sockFd = -1;
    }

    return closed;
}

bool TcpSocket::setOption(int level, int optName, int newSetting,
        std::string description)
{
    bool successful(true);
    socklen_t optlen = sizeof(newSetting);
    int worked(-1);
    int readOption(-1);

    worked = setsockopt(m_sockFd, level, optName, &newSetting, optlen);
    if (worked < 0)
    {
        if (NULL != G_MyApp)
        {
            G_MyApp->log() << AppLog::LL_ERROR << "Unable to set TCP "
                    << description << ", errno = " << errno << EndLog;
        }
        successful = false;
    }
    else
    {
        worked = getsockopt(m_sockFd, level, optName, &readOption, &optlen);
        if (worked < 0 || readOption != newSetting)
        {
            if (NULL != G_MyApp)
            {
                G_MyApp->log() << AppLog::LL_WARNING << "TCP " << description
                        << " not set properly, return val = " << worked
                        << ", value = " << readOption << ", errno = " << errno
                        << EndLog;
            }
            successful = false;
        }
    }

    return successful;
}

void TcpSocket::setBlocking()
{
    int flags = fcntl(m_sockFd, F_GETFL, 0);
    if (flags < 0)
    {
        if (NULL != G_MyApp)
        {
            G_MyApp->log() << AppLog::LL_WARNING
                    << "TcpClient : Failed to read socket flags.  Not setting to blocking: "
                    << strerror(errno) << "[" << errno << "]" << EndLog;
        }
    }
    else
    {
        int returnCode = fcntl(m_sockFd, F_SETFL, flags & ~O_NONBLOCK);
        if (returnCode < 0)
        {
            if (NULL != G_MyApp)
            {
                G_MyApp->log() << AppLog::LL_WARNING
                        << "TcpClient : Failed to set socket to blocking: "
                        << strerror(errno) << "[" << errno << "]" << EndLog;
            }
        }
    }
}

int TcpSocket::getErrorCode() const
{
    int errorCode;
    socklen_t length = sizeof(errorCode);
    int errorCodeReturn;
    int getSockOptReturn = getsockopt(m_sockFd, SOL_SOCKET, SO_ERROR,
            &errorCode, &length);
    if (getSockOptReturn >= 0)
    {
        errorCodeReturn = errorCode;
    }
    else
    {
        if (NULL != G_MyApp)
        {
            G_MyApp->log() << AppLog::LL_WARNING
                    << "TcpClient : getsockopt failed: " << strerror(errno)
                    << "[" << errno << "]" << EndLog;
        }
        errorCodeReturn = errno;
    }

    return errorCodeReturn;
}

int TcpSocket::getSockFd() const
{
    return m_sockFd;
}

void TcpSocket::setSockFd(int sockFd)
{
    m_sockFd = sockFd;
}

} /* namespace NetworkKit */
