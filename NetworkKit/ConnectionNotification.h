/**
 * \file      ConnectionNotification.h
 * \brief     Declaration of ConnectionNotification class
 * \date      2013-11-15
 * \author    Ryan O'Farrell
 */

#ifndef CONNECTIONNOTIFICATION_H_
#define CONNECTIONNOTIFICATION_H_

#include "TcpSocket.h"
#include "ConnectionStates.h"

namespace NetworkKit
{

/**
 * \brief Encapsulates a TCP network message
 * \author Ryan O'Farrell
 * \date 2013-11-15
 */
class ConnectionNotification
{
public:
    /** \brief Input source that received the message, can be used to send a reply message */
    TcpSocket * const socket;

    /** \brief State of the connection (connected or disconnected) */
    const ConnectionStates::ConnectionStatesEnum state;

    /**
     * \brief Constructor
     * \param socket the socket with a connection state change
     * \param state the connection state (connected or disconnected)
     */
    ConnectionNotification(TcpSocket * const socket, const ConnectionStates::ConnectionStatesEnum state);

    /**
     * \brief Copy constructor
     * \param other message to be copied
     */
    ConnectionNotification(ConnectionNotification const & other);

private:
    /**
     * \brief Assignment operator not supported
     */
    ConnectionNotification& operator=(ConnectionNotification const &other);

};

/**
 * \brief Formats \c ConnectionNotification for printing to screen
 * \param os output stream
 * \param p the notification
 * \return output stream including notification
 */
std::ostream & operator<<(std::ostream &os, const ConnectionNotification& p);

} /* namespace NetworkKit */
#endif /* TCPMESSAGENOTIFICATION_H_ */
