/**
 * \file RuntimeErrorException.h
 * \brief Definition of the RuntimeErrorException Class
 * \date 2012-09-24 16:49:03
 * \author Rolando J. Nieves
 */
#if !defined(EA_FE02DE74_BF4D_4a15_A3FE_8FB99F87A517__INCLUDED_)
#define EA_FE02DE74_BF4D_4a15_A3FE_8FB99F87A517__INCLUDED_

#include <exception>
#include <string>

namespace CoreKit
{
	/**
	 * \brief Exception Raised When the Foundation Experiences an Unknown Error
	 * 
	 * The RESOLVE Foundation classes are bound to raise a
	 * \c CoreKit::RuntimeErrorException exception whenever an unknown error
	 * occurs during application runtime. An unknown error is one that cannot
	 * be cataloged as none of:
	 * - Invalid Input
	 * - Precondition Not Met
	 * - Operating System Error
	 *
	 * \author Rolando J. Nieves
	 * \date 2012-09-24 16:47:47
	 */
	class RuntimeErrorException : public std::exception
	{

	public:
		/**
		 * \brief Initialize the Exception Message With The Provided Error Description
		 *
		 * The primary constructor for the \c RuntimeErrorException class
		 * initializes the exception message using the plain-text description
		 * of the error condition encountered by the RESOLVE foundation.
		 *
		 * \param errorDescription Short, plain-text description of the error
		 *                         condition encountered.
		 */
		explicit RuntimeErrorException(std::string const& errorDescription);
		/**
		 * \brief Copy the Exception Message from Another \c RuntimeErrorException Instance onto This One
		 *
		 * The \c RuntimeErrorException copy constructor makes a deep copy of
		 * the exception message in another \c OsErrorException instance onto
		 * this instance.
		 *
		 * \param other \c RuntimeErrorException instance which will serve as
		 *              the source of the copy operation.
		 */
                inline RuntimeErrorException(RuntimeErrorException const &other)
                    : m_exceptMsg(other.m_exceptMsg){};
                /**
		 * \brief Tie in to the Destructor Chain of the \c std::exception Base Class
		 */
		virtual ~RuntimeErrorException() throw();

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
#endif // !defined(EA_FE02DE74_BF4D_4a15_A3FE_8FB99F87A517__INCLUDED_)
