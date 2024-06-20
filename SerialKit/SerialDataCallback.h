/**
 * \file SerialDataCallback.h
 * \brief Definition of the SerialDataCallback Class
 * \date 2012-09-20 17:25:59
 * \author Rolando J. Nieves
 */
#if !defined(EA_45E0E591_204E_4498_98CA_1868E4564D26__INCLUDED_)
#define EA_45E0E591_204E_4498_98CA_1868E4564D26__INCLUDED_

#include "SerialDataNotification.h"

namespace SerialKit
{
	/**
	 * \brief Defines the interface for receiving serial data
	 */
	class SerialDataCallback
	{

	public:
	    /**
	     * \brief Constructor
	     */
		SerialDataCallback();

		/**
		 * \brief Destructor
		 */
		virtual ~SerialDataCallback();

		/**
		 * \brief Makes this class functor for receiving serial data notifications
		 */
		virtual void operator()(SerialDataNotification* theNotification) =0;

		/* Use default copy constructor */

	};

}
#endif // !defined(EA_45E0E591_204E_4498_98CA_1868E4564D26__INCLUDED_)
