/**
 * \file ConnectionStates.h
 * \brief Definition of the ConnectionStates Class
 * \date 2013-11-14 13:48:19
 * \author Ryan O'Farrell
 */

#ifndef CONNECTIONSTATES_H_
#define CONNECTIONSTATES_H_

namespace NetworkKit
{

/**
 * \brief Enumeration of connection states of a TCP socket
 * \author Ryan O'Farrell
 * \date 2013-11-14
 */
class ConnectionStates
{
public:
    enum ConnectionStatesEnum
    {
        DISCONNECTED = 0,
        CONNECTED,
        PENDING
    };
};
}

#endif /* CONNECTIONSTATES_H_ */
