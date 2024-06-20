/**
 * \file SerialDataCallbackT.h
 * \brief Definition of the SerialDataCallbackT Class
 * \date 2012-09-20 17:25:59
 * \author Rolando J. Nieves
 */
#if !defined(EA_B05FEFF7_F24A_4c29_B9A5_0C752F324AD7__INCLUDED_)
#define EA_B05FEFF7_F24A_4c29_B9A5_0C752F324AD7__INCLUDED_

#include "SerialDataCallback.h"

namespace SerialKit
{
    /**
     * \brief Template class for creating a \c SerialDataCallback that meets
     *        the expected interface requirements.
     * \details Can be used with a C style function or functor object.  This class
     *          does not assume a heap object and so does not handle memory de-allocation
     */
	template<class TargetType>
	class SerialDataCallbackT : public SerialKit::SerialDataCallback
	{

	public:
	    /**
	     * \brief Constructor
	     * \param callbackTarget the callback
	     */
		SerialDataCallbackT(TargetType callbackTarget)
		: m_callbackTarget(callbackTarget)
		{

		}

		/**
		 * \brief Copy constructor
		 * \param other Callback helper to copy
		 */
		SerialDataCallbackT(SerialDataCallbackT const& other)
		: m_callbackTarget(other.m_callbackTarget)
		{

		}

	    /**
	     * \brief Assignment operator
	     * \param other object to assign
	     */
	    SerialDataCallbackT& operator=(const SerialDataCallbackT& other)
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
		virtual ~SerialDataCallbackT()
		{

		}

        /**
         * \brief Delegates message to registered callback target
         * \param theNotification received TCP message
         */
		virtual void operator()(SerialDataNotification* theNotification)
		{
			m_callbackTarget(theNotification);
		}

	private:
		/** The callback function/object */
		TargetType m_callbackTarget;
	};

    /**
     * \brief Template function to create a new heap instance of this object
     * \details Used for creating a new callback that can be registered with other
     *  \c SerialKit classes
     *
     *  \param callbackTarget The callback function/object
     */
	template<class TargetType>
	SerialDataCallback* newSerialDataCallback(TargetType callbackTarget)
	{
		return new SerialDataCallbackT<TargetType>(callbackTarget);
	}
}
#endif // !defined(EA_B05FEFF7_F24A_4c29_B9A5_0C752F324AD7__INCLUDED_)
