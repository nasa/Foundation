/**
 * \file      NetworkKit.h
 * \brief     Declaration of NetworkKit class
 * \date      2013-01-10
 * \author    Ryan O'Farrell
 */

#ifndef NETWORKKIT_H_
#define NETWORKKIT_H_

/**
 * \brief Collection of classes that facilitate raw network connectivity for Foundation applications.
 * \details Provides classes for communication over TCP with sockets.
 *          Classes are provided to create a TCP server or client and to send
 *          and receive messages via the connection
 */
namespace NetworkKit
{

}

#include "TcpClient.h"
#include "TcpMessageCallback.h"
#include "TcpMessageCallbackT.h"
#include "ConnectionCallback.h"
#include "ConnectionCallbackT.h"
#include "TcpMessageInputSource.h"
#include "TcpServerInputSource.h"
#include "TcpMessageNotification.h"
#include "UdpPacketDistribution.hh"
#include "UdpPacketNotification.hh"
#include "UdpSocket.hh"

#endif /* NETWORKKIT_H_ */
