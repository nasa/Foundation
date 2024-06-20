/**
 * \file OsErrorException.h
 * \brief Definition of the OsErrorException Class
 * \date 2012-09-24 14:42:02
 * \author Rolando J. Nieves
 */
#if !defined(EA_3D9E0BA0_8EB8_446a_9D74_583876362B47__INCLUDED_)
#define EA_3D9E0BA0_8EB8_446a_9D74_583876362B47__INCLUDED_

#include <exception>
#include <string>

namespace CoreKit
{
	/**
	 * \brief Exception Raised when the Foundation Classes Encounter an Operating System Error
	 * 
	 * The RESOLVE Foundation classes are liable to raise a
	 * \c CoreKit::OsErrorException exception when a call to the operating
	 * system fails or reports an error.
	 *
	 * \author Rolando J. Nieves
	 * \date 24-Sep-2012 2:12:20 PM
	 */
	class OsErrorException : public std::exception
	{

	public:
		/**
		 * \brief Initialize the Exception Message with the Pertinent Information
		 *
		 * The \c OsErrorException main constructor initializes the exception
		 * message with the name of the operating system call that failed and
		 * the error code reported by the operating system call.
		 *
		 * \param osCall Operating system call that reported the error.
		 * \param errorNumber Error code reported by the call (as reported via
		 *                    \c errno ).
		 */
		OsErrorException(std::string const& osCall, int errorNumber);
		/**
		 * \brief Copy the Exception Message from Another \c OsErrorException Instance onto This One
		 *
		 * The \c OsErrorException copy constructor makes a deep copy of the
		 * exception message in another \c OsErrorException instance onto this
		 * instance.
		 *
		 * \param other \c OsErrorException instance which will serve as the
		 *              source of the copy operation.
		 */
		inline OsErrorException(OsErrorException const& other)
		: m_exceptMsg(other.m_exceptMsg)
		{}
		/**
		 * \brief Tie in to the Destructor Chain of the \c std::exception Base Class
		 */
		virtual ~OsErrorException() throw();

		/**
		 * \brief Provide the Exception Message as a Character Array
		 *
		 * \return Exception message converted to a character array.
		 */
		virtual const char* what() const throw();

	private:

		/**
		 * \brief Exception Message String
		 */
		std::string m_exceptMsg;
	};

}
#endif // !defined(EA_3D9E0BA0_8EB8_446a_9D74_583876362B47__INCLUDED_)
