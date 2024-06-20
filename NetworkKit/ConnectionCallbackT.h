/**
 * \file      ConnectionCallbackT.h
 * \brief     Declaration of ConnectionCallbackT class
 * \date      2012-12-13
 * \author    Ryan O'Farrell
 */

#ifndef CONNECTIONCALLBACKT_H_
#define CONNECTIONCALLBACKT_H_

#include "ConnectionCallback.h"

namespace NetworkKit
{

/**
 * \brief Template class for creating a \c ConnectionCallback that meets
 *        the expected interface requirements.
 * \details Can be used with a C style function or functor object.  This class
 *          does not assume a heap object and so does not handle memory de-allocation
 * \tparam TargetType type of callback target
 * \author Ryan O'Farrell
 * \date 2012-12-13
 */
template<typename TargetType>
class ConnectionCallbackT: public NetworkKit::ConnectionCallback
{
public:
    /**
     * \brief Constructor
     * \param callbackTarget theCallback
     */
    ConnectionCallbackT(TargetType callbackTarget) :
            m_callbackTarget(callbackTarget)
    {

    }

    /**
     * \brief Copy constructor
     * \param other to copy
     */
    ConnectionCallbackT(ConnectionCallbackT const& other) :
            m_callbackTarget(other.m_callbackTarget)
    {

    }


    /**
     * \brief Assignment operator
     * \param other object to assign
     */
    ConnectionCallbackT& operator=(const ConnectionCallbackT& other)
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
    virtual ~ConnectionCallbackT()
    {

    }

    /**
     * \brief Delegates message to registered callback target
     * \param notification received TCP message
     */
    virtual void operator()(ConnectionNotification *notification)
    {
        m_callbackTarget(notification);
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
ConnectionCallback* newConnectionCallback(TargetType callbackTarget)
{
    return new ConnectionCallbackT<TargetType>(callbackTarget);
}

} /* namespace NetworkKit */
#endif /* TCPMESSAGECALLBACKT_H_ */
