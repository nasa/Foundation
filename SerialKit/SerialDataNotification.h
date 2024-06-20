/**
 * \file SerialDataNotification.h
 * \brief Definition of the SerialDataNotification Class
 * \date 2012-09-20 17:25:59
 * \author Rolando J. Nieves
 */
#if !defined(EA_B983EF8D_1A3A_47ef_9CF2_4C16FBCFE4B6__INCLUDED_)
#define EA_B983EF8D_1A3A_47ef_9CF2_4C16FBCFE4B6__INCLUDED_

#include <time.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <ostream>
#include "CoreKit/ByteVector.h"

namespace SerialKit
{
	/**
	 * \brief Encapsulates a set of received serial data
	 */
	struct SerialDataNotification
	{

	public:
	    /** Time data acquired */
		timespec const& acqTime;
		/** data received */
		CoreKit::FixedByteVector const& serialData;
		/** Name of serial port that delivered data */
		std::string const& serialPort;

		/**
		 * \brief Constructor
		 * \param theSerialPort where data was received
		 * \param theAcqTime time received
		 * \param theSerialData data as bytes
		 */
		SerialDataNotification(std::string const& theSerialPort, timespec const& theAcqTime, CoreKit::FixedByteVector const& theSerialData);

		SerialDataNotification(SerialDataNotification const& other);

	private:
		timespec m_acqTime;
		CoreKit::FixedByteVector m_serialData;
		std::string m_serialPort;

		SerialDataNotification(size_t bufferSize);

		friend class SerialIo;
	};

/**
 * \brief Formats \c SerialDataNotification for printing to screen
 * \param os output stream
 * \param p the notification
 * \return output stream including notification
 */
std::ostream & operator<<(std::ostream &os, const SerialDataNotification& p);

}
#endif // !defined(EA_B983EF8D_1A3A_47ef_9CF2_4C16FBCFE4B6__INCLUDED_)
