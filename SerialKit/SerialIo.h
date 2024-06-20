/**
 * \file SerialIo.h
 * \brief Definition of the SerialIo Class
 * \date 20-Sep-2012 5:26:00 PM
 * \author rnieves
 * \copyright National Aeronautics and Space Administration.
 */
#if !defined(EA_82D8F064_CE60_4321_B7D6_56DE4D6D77C3__INCLUDED_)
#define EA_82D8F064_CE60_4321_B7D6_56DE4D6D77C3__INCLUDED_

#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <string>
#include <vector>
#include <algorithm>
#include <termios.h>

#include "CoreKit/InputSource.h"
#include "CoreKit/InterruptListener.h"
#include "CoreKit/RunLoop.h"
#include "CoreKit/factory.h"
#include "CoreKit/AppLog.h"
#include "SerialDataCallback.h"
#include "CoreKit/AppLog.h"

/** Defines the maximum size of a single serial transmission */
#define RF_SIO_DATA_CHUNK_MAX_SIZE (64u)

namespace SerialKit
{
	/**
	 *  \brief Provides access to a single serial port for sending and receiving data
	 *  \details  Callbacks can be registered with \c registerSerialDataCallback.
	 *  Any registered callbacks will be owned by the class and destroyed when the class is destroyed.
	 *  This class is single threaded and uses the provided \c CoreKit::RunLoop to process
	 *  data received from the port.
	 */
	class SerialIo : public CoreKit::InputSource, public CoreKit::InterruptListener
	{
		RF_CK_FACTORY_COMPATIBLE(SerialIo);
	public:
	    /**
	     * \brief Constructor
	     * \param serialPort name of the serial port
	     * \param hostRunLoop the \c RunLoop used for receiving data
	     * \param i_log the Application logger
	     */
		SerialIo(std::string const& serialPort, CoreKit::RunLoop* hostRunLoop, CoreKit::AppLog *i_log);

		/**
		 * \brief Destructor
		 */
		virtual ~SerialIo();

		int fileDescriptor() const;
		InterruptListener* interruptListener() const;
		void inputAvailableFrom(InputSource* theInputSource);

		/**
		 * \brief Reads the data from the serial port
		 * \details Called when the \c RunLoop notifies that there is available data and provided
		 * automatically to callbacks.  Thres is no need for clients to call this function manually.
		 *
		 * \param dataBuffer by-ref paramater for returning read data
		 */
		template<class VectorType>
		void readAvailableData(VectorType& dataBuffer)
		{
			uint8_t readChunk[RF_SIO_DATA_CHUNK_MAX_SIZE];
			ssize_t readResult = -1;
			size_t readAmount = 0u;
			typename VectorType::iterator insertPos = dataBuffer.begin();

			memset(readChunk, 0x00, sizeof(readChunk));

			readAmount = std::min(sizeof(readChunk), dataBuffer.capacity() - dataBuffer.size());

			while ((readAmount > 0u) && ((readResult = read(m_serialPortFd, readChunk, readAmount)) > 0))
			{
				dataBuffer.resize(dataBuffer.size() + readResult, uint8_t(0x00));
				insertPos = dataBuffer.end() - readResult;
				std::copy(&readChunk[0], &readChunk[readResult], insertPos);
				readAmount = std::min(sizeof(readChunk), dataBuffer.capacity() - dataBuffer.size());
			}
		}


		/**
		 * \brief Registers a new callback for any serial data received
		 * \details This class will take ownership of theCallback and delete its memory in the destructor.
		 * \param theCallback callback instance (heap allocated)
		 * \throw CoreKit::PreconditionNotMetException if theCallback is NULL
		 */
		void registerSerialDataCallback(SerialDataCallback* theCallback);

		/**
		 * \brief Sends data over the serial port
		 * \param dataToSend as bytes
		 * \return 0 on success, -1 if not all bytes are written
		 */
		template<typename VectorType>
		int sendData(VectorType const& dataToSend)
		{
			uint8_t writeChunk[RF_SIO_DATA_CHUNK_MAX_SIZE];
			ssize_t writeResult = -1;
			typename VectorType::const_iterator currPos = dataToSend.begin();
			int chunkIdx = 0;
			int retCode = 0;

			while (currPos != dataToSend.end())
			{
				chunkIdx = 0;
				memset(writeChunk, 0x00, sizeof(writeChunk));
				while ((chunkIdx < sizeof(writeChunk)) && (currPos != dataToSend.end()))
				{
					writeChunk[chunkIdx] = (*currPos);
					chunkIdx++;
					++currPos;
				}

				writeResult = this->writeWithBlock(&writeChunk[0], chunkIdx, 3.0f /* TODO: Make this configurable later. */);
				if (writeResult != chunkIdx)
				{
				    if (NULL != m_log)
				    {
		                *m_log << CoreKit::AppLog::LL_WARNING << "Unable to write data to serial port "
		                	   << m_serialPort << " with fd = " << m_serialPortFd << ":" << strerror(errno) << CoreKit::EndLog;
				    }
				    retCode = -1;
				}
			}

			return retCode;
		}

		/**
		 * \brief Initializes serial port for communication
		 * \details This must be called before receiving data.  After this is called, any incoming data will
		 * be processed.
		 * \throws CoreKit::OsErrorException if open of serial port fails
		 * \throws CoreKit::PreconditionNotMetException if already started
		 */
		void startSerialIo();

		/**
		 * \brief Initializes serial port for communication and configures
		 * \details This must be called before receiving data.  After this is called, any incoming data will
		 * be processed.
		 * \param baudRate the baud rate for the port
		 * \param raw if true, this port is configured in raw mode
		 * \throws CoreKit::OsErrorException if open of serial port fails
		 * \throws CoreKit::PreconditionNotMetException if already started
		 */
		void startSerialIo(speed_t baudRate, bool raw);

		/**
         * \brief Terminates serial port for communication
         * \details Can be called manually, but is also automatically called upon class destruction
         */
		void stopSerialIo();

		virtual void fireCallback();

		void bufferData(size_t bufferSize);

	private:
		/** Application run loop*/
		CoreKit::RunLoop *m_runLoop;
		/** Application logger */
		CoreKit::AppLog *m_log;
		/** Container for data notification callbacks */
		std::vector<SerialDataCallback*> m_callbacks;
		/** Serial Port file descriptor */
		int m_serialPortFd;
		/** Serial port name */
		std::string m_serialPort;
		SerialDataNotification *m_prototypeNotif;
		bool m_buffering;

		/**
		 * \brief Closes the serial port if open
		 */
		void closeSerialPort();

		/**
		 * \brief Attempts to open the serial port
		 * \throws CoreKit::OsErrorException if open fails
		 * \throws CoreKit::PreconditionNotMetExecption if already started
		 */
		void openSerialPort();

		int writeWithBlock(uint8_t const* data, size_t size, float timeout);

        /**
         * \brief Private copy constructor.  Undefined to prevent copying.
         * \param other object to copy
         */
        SerialIo(const SerialIo& other);

        /**
         * \brief Private assignment operator.  Undefined to prevent copying.
         * \param other object to assign
         */
        SerialIo& operator=(const SerialIo& other);

	};
}
#endif // !defined(EA_82D8F064_CE60_4321_B7D6_56DE4D6D77C3__INCLUDED_)
