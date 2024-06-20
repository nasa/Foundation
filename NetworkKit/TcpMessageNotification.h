/**
 * \file      TcpMessageNotification.h
 * \brief     Declaration of TcpMessageNotification class
 * \date      2012-12-13
 * \author    Ryan O'Farrell
 */

#ifndef TCPMESSAGENOTIFICATION_H_
#define TCPMESSAGENOTIFICATION_H_

#include <time.h>
#include <string>
#include <ostream>
#include <vector>
#include <stdint.h>

#include <CoreKit/ByteVector.h>

#include "TcpSocket.h"

namespace NetworkKit
{
/**
 * \brief Encapsulates a TCP network message
 * \author Ryan O'Farrell
 * \date 2012-12-13
 */
class TcpMessageNotification
{
public:
    /** \brief The time when the message was acquired from the network */
    timespec const& acqTime;

    /** \brief Application layer message received */
    CoreKit::FixedByteVector const& message;

    /** \brief Input source that received the message, can be used to send a reply message */
    const TcpSocket *const socket;

    /**
     * \brief Constructor
     * \param theAcqTime time acquired
     * \param thePayload message payload
     * \param theSocket the socket that received the message
     */
    TcpMessageNotification(timespec const& theAcqTime,
            CoreKit::FixedByteVector const& thePayload,
            const TcpSocket *const theSocket);

    /**
     * \brief Copy constructor
     * \param other message to be copied
     */
    TcpMessageNotification(TcpMessageNotification const & other);

private:
    timespec m_acqTime;

    CoreKit::FixedByteVector m_message;

    TcpMessageNotification(size_t bufferSize,
            const TcpSocket *const socket);

    friend class TcpMessageInputSource;

    /**
     * \brief Assignment operator not supported
     */
    TcpMessageNotification& operator=(TcpMessageNotification const &other);

};

/**
 * \brief Formats \c TcpMessageNotification for printing to screen
 * \param os output stream
 * \param p the notification
 * \return output stream including notification
 */
std::ostream & operator<<(std::ostream &os, const TcpMessageNotification& p);

} /* namespace NetworkKit */
#endif /* TCPMESSAGENOTIFICATION_H_ */
