/**
 * \file UdpSocket.cpp
 * \brief Contains the implementation of the \c NetworkKit::UdpSocket class.
 * \date 2019-08-15 12:49:58
 * \author Rolando J. Nieves
 */

#include <cerrno>
#include <stdexcept>
#include <system_error>
#include <sstream>
#include <unistd.h>
#include <arpa/inet.h>

#include "UdpSocket.hh"


using std::string;
using std::stringstream;
using std::generic_category;
using std::error_code;
using std::runtime_error;
using CoreKit::InterruptListener;

namespace NetworkKit
{

UdpSocket::UdpSocket(std::string const& uxPath):
    m_selectedFamily(AF_UNIX),
    m_socketFd(-1),
    m_listener(nullptr)
{
    memset(&m_sockaddrIp, 0x00, sizeof(m_sockaddrIp));
    memset(&m_sockaddrUn, 0x00, sizeof(m_sockaddrUn));
    m_sockaddrUn.sun_family = m_selectedFamily;
    strncpy(
        &m_sockaddrUn.sun_path[0],
        uxPath.c_str(),
        sizeof(m_sockaddrUn.sun_path) - 1u
    );
}


UdpSocket::UdpSocket(std::string const& ifAddr, int port):
    m_selectedFamily(AF_INET),
    m_socketFd(-1),
    m_listener(nullptr)
{
    memset(&m_sockaddrUn, 0x00, sizeof(m_sockaddrUn));
    memset(&m_sockaddrIp, 0x00, sizeof(m_sockaddrIp));
    m_sockaddrIp.sin_family = m_selectedFamily;
    m_sockaddrIp.sin_port = htons(port);
    inet_pton(AF_INET, ifAddr.c_str(), &m_sockaddrIp.sin_addr);
}


UdpSocket::~UdpSocket()
{
    this->terminate();
}


void
UdpSocket::initialize(InterruptListener *listener)
{
    if (nullptr == listener)
    {
        throw runtime_error("UdpSocket::initialize() passed nullptr interrupt listener instance.");
    }
    
    m_socketFd = socket(
        m_selectedFamily,
        SOCK_DGRAM,
        0
    );

    if (-1 == m_socketFd)
    {
        stringstream errorMsg;
        errorMsg
            << "Could not create UDP socket: "
            << error_code(errno, generic_category()).message();
        throw runtime_error(errorMsg.str());
    }

    struct sockaddr *theAddr = nullptr;
    socklen_t addrLen = 0;

    if (AF_UNIX == m_selectedFamily)
    {
        theAddr = reinterpret_cast< struct sockaddr* >(&m_sockaddrUn);
        addrLen = sizeof(m_sockaddrUn);
    }
    else if (AF_INET == m_selectedFamily)
    {
        theAddr = reinterpret_cast< struct sockaddr* >(&m_sockaddrIp);
        addrLen = sizeof(m_sockaddrIp);
    }

    int bindResult = bind(
        m_socketFd,
        theAddr,
        addrLen
    );

    if (-1 == bindResult)
    {
        close(m_socketFd);
        m_socketFd = -1;
        stringstream errorMsg;
        errorMsg
            << "Could not bind UDP socket to address: "
            << error_code(errno, generic_category()).message();
        throw runtime_error(errorMsg.str());
    }

    m_listener = listener;
}


void
UdpSocket::terminate()
{
    if (m_socketFd != -1)
    {
        close(m_socketFd);
        m_socketFd = -1;
        if (AF_UNIX == m_selectedFamily)
        {
            unlink(m_sockaddrUn.sun_path);
        }
    }

    m_listener = nullptr;
}


string
UdpSocket::uxPath() const
{
    string result;

    if (AF_UNIX == m_selectedFamily)
    {
        result = m_sockaddrUn.sun_path;
    }

    return result;
}

string
UdpSocket::ipAddress() const
{
    char addrArea[INET_ADDRSTRLEN];

    memset(&addrArea[0], 0x00, INET_ADDRSTRLEN);

    if (AF_INET == m_selectedFamily)
    {
        inet_ntop(
            AF_INET,
            &m_sockaddrIp.sin_addr,
            addrArea,
            INET_ADDRSTRLEN
        );
    }

    return addrArea;
}

int
UdpSocket::port() const
{
    int result = -1;

    if (AF_INET == m_selectedFamily)
    {
        result = ntohs(m_sockaddrIp.sin_port);
    }

    return result;
}


int
UdpSocket::fileDescriptor() const
{
    return m_socketFd;
}


InterruptListener*
UdpSocket::interruptListener() const
{
    return m_listener;
}


void
UdpSocket::fireCallback()
{
    m_listener->inputAvailableFrom(this);
}

} // end namespace NetworkKit

// vim: set ts=4 sw=4 expandtab:
