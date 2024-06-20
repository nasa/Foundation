/**
 * \file      TcpMessageCallbackT.h
 * \brief     Declaration of TcpMessageCallbackT class
 * \date      2012-12-13
 * \author    Ryan O'Farrell
 * \copyright National Aeronautics and Space Administration
 */

#ifndef TCPMESSAGECALLBACKT_H_
#define TCPMESSAGECALLBACKT_H_

#include "TcpMessageCallback.h"

namespace NetworkKit
{

/**
 * \brief Template class for creating a \c TcpMessageCallback that meets
 *        the expected interface requirements.
 * \details Can be used with a C style function or functor object.  This class
 *          does not assume a heap object and so does not handle memory de-allocation
 * \tparam TargetType type of callback target
 * \author Ryan O'Farrell
 * \date 2012-12-13
 */
template<typename TargetType>
class TcpMessageCallbackT: public NetworkKit::TcpMessageCallback
{
public:
    /**
     * \brief Constructor
     * \param callbackTarget theCallback
     */
    TcpMessageCallbackT(TargetType callbackTarget) :
            m_callbackTarget(callbackTarget)
    {

    }

    /**
     * \brief Copy constructor
     * \param other to copy
     */
    TcpMessageCallbackT(TcpMessageCallbackT const& other) :
            m_callbackTarget(other.m_callbackTarget)
    {

    }


    /**
     * \brief Assignment operator
     * \param other object to assign
     */
    TcpMessageCallbackT& operator=(const TcpMessageCallbackT& other)
    {
        if(this != &other)
        {
           m_callbackTarget = other.m_callbackTarget;
        }

        return *this;
    }

    /**
     * \brief Destructor
     */
    virtual ~TcpMessageCallbackT()
    {

    }

    /**
     * \brief Delegates message to registered callback target
     * \param theNotification received TCP message
     */
    virtual void operator()(TcpMessageNotification* theNotification)
    {
        m_callbackTarget(theNotification);
    }

private:

    /** the callback instance */
    TargetType m_callbackTarget;
};

/**
 * \brief Template function to create a new heap instance of this object
 * \details Used for creating a new callback that can be registered with other
 *  \c NetworkKit classes
 *
 *  \param callbackTarget The callback function/object
 */
template<class TargetType>
TcpMessageCallback* newTcpMessageCallback(TargetType callbackTarget)
{
    return new TcpMessageCallbackT<TargetType>(callbackTarget);
}

} /* namespace NetworkKit */
#endif /* TCPMESSAGECALLBACKT_H_ */
