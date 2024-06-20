/**
 * \file      ConnectionCallback.h
 * \brief     Declaration of ConnectionCallback class
 * \date      2013-01-23
 * \author    Ryan O'Farrell
 */

#ifndef CONNECTIONCALLBACK_H_
#define CONNECTIONCALLBACK_H_

#include "ConnectionStates.h"
#include "ConnectionNotification.h"

namespace NetworkKit
{

/**
 * \brief Interface for receiving a callback when a new client connects to TCP server.
 * \author Ryan O'Farrell
 * \date 2013-01-23
 */
class ConnectionCallback
{
public:
    /**
     * \brief Constructor
     */
    ConnectionCallback();

    /**
     * \brief Destructor
     */
    virtual ~ConnectionCallback();

    /**
     * \brief Functor operator for handling new connection
     * \param notification the new connection
     */
    virtual void operator()(ConnectionNotification *notification) = 0;
};

} /* namespace NetworkKit */
#endif /* CONNECTIONCALLBACK_H_ */
