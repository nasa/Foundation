/**
 * \file InvalidInputException.h
 * \brief Definition of the InvalidInputException Class
 * \date 2012-09-24 16:48:45
 * \author Rolando J. Nieves
 */
#if !defined(EA_96516C59_9303_4509_9EA6_C7E477301CC0__INCLUDED_)
#define EA_96516C59_9303_4509_9EA6_C7E477301CC0__INCLUDED_

#include <exception>
#include <string>

namespace CoreKit
{
	/**
	 * \brief Exception Raised When the Foundation Classes Receive Invalid Input
	 *
	 * The RESOLVE Foundation classes are liable to raise an
	 * \c CoreKit::InvalidInputException exception whenever the application
	 * logic provides the foundation with faulty data. The source of this
	 * faulty data may be the application logic itself or provided by an
	 * external source (e.g., invalid command line parameters provided by the
	 * operating system).
	 * 
	 * \author Rolando J. Nieves
	 * \date 24-Sep-2012 4:47:08 PM
	 */
	class InvalidInputException : public std::exception
	{

	public:
		/**
		 * \brief Initialize the Exception Message With Details Regarding the Invalid Input
		 *
		 * The primary constructor of the \c InvalidInputException class takes
		 * in a description of the input that generated the condition as well
		 * as the input value itself that was at fault.
		 *
		 * \param inputDescription Short, plain English description of the
		 *                         input parameter that generated the fault.
		 * \param inputValue Character string representation of the actual
		 *                   input value that generated the fault.
		 */
		InvalidInputException(std::string const& inputDescription, std::string const& inputValue);
		/**
		 * \brief Copy the Exception Message from Another Instance onto This Instance
		 *
		 * The copy constructor of the \c InvalidInputException class
		 * performs a deep copy of the exception message built by another
		 * instance of this class onto this instance's exception message.
		 *
		 * \param other Other \c InvalidInputException instance whose exception
		 *              message should be copied onto this one.
		 */
		inline InvalidInputException(InvalidInputException const& other)
		: m_exceptMsg(other.m_exceptMsg)
		{};
		/**
		 * \brief Tie in to the Destructor Chain of the \c std::exception Base Class
		 */
		virtual ~InvalidInputException() throw();

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
#endif // !defined(EA_96516C59_9303_4509_9EA6_C7E477301CC0__INCLUDED_)
