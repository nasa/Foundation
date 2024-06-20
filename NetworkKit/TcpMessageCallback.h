/**
 * \file      TcpMessageCallback.h
 * \brief     Declaration of TcpMessageCallback class
 * \date      2012-12-13
 * \author    Ryan O'Farrell
 */

#ifndef TCPMESSAGECALLBACK_H_
#define TCPMESSAGECALLBACK_H_

#include "TcpMessageNotification.h"

namespace NetworkKit
{

/**
 * \brief Defines the interface for callbacks to receive TCP messages
 * \author Ryan O'Farrell
 * \date 2012-12-13
 */
class TcpMessageCallback
{
public:
    /**
     * \brief Constructor
     */
    TcpMessageCallback();

    /**
     * \brief Destructor
     */
    virtual ~TcpMessageCallback();

    /**
     * \brief Callback method passed TCP message
     */
    virtual void operator()(TcpMessageNotification *theNotification) = 0;
};

} /* namespace NetworkKit */
#endif /* TCPMESSAGECALLBACK_H_ */
