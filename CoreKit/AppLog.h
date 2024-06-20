/**
 * \file AppLog.h
 * \brief Definition of the AppLog Class
 * \date 2012-09-25 11:48:27
 * \author Rolando J. Nieves
 */
#if !defined(EA_C35AB279_B9B1_4773_8F53_6451BE9F7BE9__INCLUDED_)
#define EA_C35AB279_B9B1_4773_8F53_6451BE9F7BE9__INCLUDED_

#include <stdint.h>
#include <string>
#include <sstream>

#include "factory.h"

namespace CoreKit
{
	/**
	 * \brief Log Service Class for Framework Applications
	 *
	 * The \c CoreKit::AppLog class models a message logging service that wraps
	 * the native operating system logging functionality, configures it in a
	 * common way, and exposes the functionality to framework application
	 * developers.
	 * 
	 * \author Rolando J. Nieves
	 * \date 25-Sep-2012 11:33:07 AM
	 */
	class AppLog
	{
		RF_CK_FACTORY_COMPATIBLE(AppLog);
	public:
		/**
		 * \brief Log Severity Level for a Message
		 */
		enum Level { LL_DEBUG = 0, LL_INFO, LL_WARNING, LL_ERROR };
		/**
		 * \brief Placeholder that Indicates the End of a Log Message
		 */
		struct End {};

		/**
		 * \brief Initialize the Operating System Logging Services
		 *
		 * \param appName String that should be used to identify the
		 *                log messages generated by this \c AppLog instance.
		 * \param doStdErr Echo log message output to \c stderr along with the
		 *                 \c syslog() output. Defaults to \c true.
		 */
		AppLog(std::string const& appName, bool doStdErr = true);
		/**
		 * \brief Terminate the Operating System Logging Services
		 */
		virtual ~AppLog();

		/**
		 * \brief Access the Log Message Identifier Used by This \c AppLog Instance
		 *
		 * \return Log Message identifier used by this \c AppLog instance
		 */
		inline std::string const& appName() const { return m_appName; }
		/**
		 * \brief Access the Minimum Log Level Used by This \c AppLog Instance
		 *
		 * \return Minimum log level used by this \c AppLog instance.
		 */
		inline Level minLevel() const { return m_minLevel; }
		/**
		 * \brief Alter the Minimum Log Level Used by This \c AppLog Instance
		 *
		 * \param minLevel Minimum log level that this \c AppLog instance
		 *                 should use.
		 */
		inline void setMinLevel(Level minLevel) { m_minLevel = minLevel; }

		/**
		 * \brief Add a Value to the Current Log Message
		 *
		 * \param streamInVal Value to add to the current log message.
		 *
		 * \return This \c AppLog instance after adding the value.
		 */
		template<class StreamInType> AppLog& operator <<(StreamInType streamInVal)
		{ m_currentLogMsg << streamInVal; return *this; }

	private:

		/**
		 * \brief Identification String to go Along With All Log Messages
		 */
		std::string m_appName;
		/**
		 * \brief Character Stream Containing the Current Log Message
		 */
		std::stringstream m_currentLogMsg;
		/**
		 * \brief Minimum Severity Level to Obey when Publishing Log Messages
		 */
		Level m_minLevel;
		/**
		 * \brief Severity Level of the Current Log Message
		 */
		Level m_logLevel;
        /**
         * \brief In addition to syslog, if available, log to stderr.
         */
        bool m_doStdErr;
	};

	/**
	 * \brief Global Constant Used as Shorthand for \c AppLog::End
	 */
	extern const AppLog::End EndLog;

	/**
	 * \brief Set the Severity Level of the Current Log Message
	 *
	 * This specialization of the AppLog::operator<<() template function
	 * handles the altering of the log level accordingly.
	 *
	 * \param logLevel Severity level of the current log message.
	 *
	 * \return This \c AppLog instance after setting the severity level.
	 */
	template<> AppLog& AppLog::operator << <AppLog::Level>(AppLog::Level logLevel);

	/**
	 * \brief End the Current Log Message
	 *
	 * This specialization of the \c AppLog::operator<<() that takes an
	 * instance of the log message end marker ( \c AppLog::End ) takes the
	 * current log message that has been built via other calls to the
	 * \c AppLog::operator<<() and sends it via the operating system specific
	 * log message service.
	 *
	 * \param endMark Instance of the \c AppLog::End type.
	 *
	 * \return This \c AppLog instance after flushing out the current log
	 *         message.
	 *
	 * \post
	 * The current log message is empty.
	 */
	template<> AppLog& AppLog::operator << <AppLog::End>(AppLog::End endMark);

	/**
	 * \brief Create a Formatted String from a Single-Precision Floating Point Value
	 *
	 * The \c format() function takes in a \c printf() like formatting string
	 * and a single-precision floating point value and creates a character
	 * string from the value that matches the format specified.
	 *
	 * \param formatStr \c printf() like format string to use on the floating
	 *                  point value.
	 * \param floatVal Single-precision floating point value to convert to a
	 *                 character string.
	 *
	 * \return A character string with the floating point value formatted per
	 *         the format string specified.
	 */
	std::string format(std::string const& formatStr, float floatVal);
	/**
	 * \brief Create a Formatted String from a Double-Precision Floating Point Value
	 *
	 * The \c format() function takes in a \c printf() like formatting string
	 * and a double-precision floating point value and creates a character
	 * string from the value that matches the format specified.
	 *
	 * \param formatStr \c printf() like format string to use on the floating
	 *                  point value.
	 * \param doubleVal Double-precision floating point value to convert to a
	 *                  character string.
	 *
	 * \return A character string with the floating point value formatted per
	 *         the format string specified.
	 */
	std::string format(std::string const& formatStr, double doubleVal);
	/**
	 * \brief Create a Formatted String from an Integer Value
	 *
	 * The \c format() function takes a \c printf() like formatting string and
	 * a signed integer value and creates a character string from the value
	 * that matches the format specified.
	 *
	 * \param formatStr \c printf() like format string to use on the floating
	 *                  point value.
	 * \param intVal Signed integer value to convert to a character string.
	 *
	 * \return A character string with the signed integer value formatted per
	 *         the format string specified.
	 */
	std::string format(std::string const& formatStr, int intVal);
	/**
	 * \brief Create a Formatted String from an Unsigned 32-bit Integer Value
	 *
	 * The \c format() function takes a \c printf() like formatting string and
	 * an unsigned 32-bit integer value and creates a character string from the
	 * value that matches the format specified.
	 *
	 * \param formatStr \c printf() like format string to use on the unsigned
	 *                  32-bit integer value.
	 * \param uint32Val Unsigned 32-bit integer value to convert to a character
	 *                  string.
	 *
	 * \return A character string with the unsigned 32-bit integer value
	 *         formatted per the format string specified.
	 */
	std::string format(std::string const& formatStr, uint32_t uint32Val);
	/**
	 * \brief Create a Formatted String from a Signed 16-bit Integer Value
	 *
	 * The \c format() function takes a \c printf() like formatting string and
	 * a signed 16-bit integer value and creates a character string from the
	 * value that matches the format specified.
	 *
	 * \param formatStr \c printf() like format string to use on the signed
	 *                  16-bit integer value.
	 * \param int16Val Signed 16-bit integer value to convert to a character
	 *                 string.
	 *
	 * \return A character string with the signed 16-bit integer value
	 *         formatted per the format string specified.
	 */
	std::string format(std::string const& formatStr, int16_t int16Val);
	/**
	 * \brief Create a Formatted String from an Unsigned 16-bit Integer Value
	 *
	 * The \c format(std::string,uint16_t) function takes a \c printf() like
	 * formatting string and an unsigned 16-bit value and creates a character
	 * string from the value that matches the format specified.
	 *
	 * \param formatStr \c printf() like format string to use on the unsigned
	 *                  16-bit integer value.
	 * \param uint16Val Unsigned 16-bit integer value to convert to a character
	 *                  string.
	 *
	 * \return A character string with the unsigned 16-bit integer value
	 *         formatted per the format string specified.
	 */
	std::string format(std::string const& formatStr, uint16_t uint16Val);
	/**
	 * \brief Create a Formatted String from an Unsigned 8-bit Integer Value
	 *
	 * The \c format(std::string,uint8_t) function takes a \c printf() like
	 * formatting string and an unsigned 8-bit value and creates a character
	 * string from the value that matches the format specified.
	 *
	 * \param formatStr \c printf() like format string to use on the unsigned
	 *                  8-bit integer value.
	 * \param byteVal Unsigned 8-bit integer value to convert to a character
	 *                string.
	 *
	 * \return A character string with the unsigned 8-bit integer value
	 *         formatted per the format string specified.
	 */
	std::string format(std::string const& formatStr, uint8_t byteVal);
}
#endif // !defined(EA_C35AB279_B9B1_4773_8F53_6451BE9F7BE9__INCLUDED_)
