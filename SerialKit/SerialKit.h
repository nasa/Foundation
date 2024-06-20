/**
 * \file SerialKit.h
 * \date 2012-10-22
 * \author Rolando J. Nieves
 */

#ifndef SERIALKIT_H_
#define SERIALKIT_H_

/**
 * SerialKit Headers
 */
#include <SerialKit/SerialDataCallback.h>
#include <SerialKit/SerialDataCallbackT.h>
#include <SerialKit/SerialDataNotification.h>
#include <SerialKit/SerialIo.h>
#include <SerialKit/SerialUtilities.h>

/**
 * \brief Provides classes that facilitate serial port connectivity to Foundation applications.
 * \details A new instance of the SerialIo class can be created for each
 * serial port being used for communication.  This class uses a provided
 * \c CoreKit::RunLoop from within the application to process incoming
 * serial data.  Classes are also provided for creating and registering a
 * callback when serial data arrives, and extracting the serial data along
 * with the source and time of arrival.
 */
namespace SerialKit
{

}
#endif /* SERIALKIT_H_ */
