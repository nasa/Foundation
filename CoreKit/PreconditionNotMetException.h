/**
 * \file PreconditionNotMetException.h
 * \brief Definition of the PreconditionNotMetException Class
 * \date 2012-09-24 14:15:11
 * \author Rolando J. Nieves
 */
#if !defined(EA_3DA7E627_BC2E_413c_8603_D1B27644B44B__INCLUDED_)
#define EA_3DA7E627_BC2E_413c_8603_D1B27644B44B__INCLUDED_

#include <string>
#include <exception>

namespace CoreKit
{
	/**
	 * \brief Exception Raised when Foundation Classes Detect a Precondition Violation
	 *
	 * The RESOLVE Foundation classes are liable to raise a
	 * \c CoreKit::PreconditionNotMetException exception when a pre-condition
	 * in a class method is not met.
	 * 
	 * \author Rolando J. Nieves
	 * \date 2012-09-24 14:10:33
	 */
	class PreconditionNotMetException : public std::exception
	{

	public:
		/**
		 * \brief Initialize the Exception Message with the Pertinent Information
		 *
		 * The main constructor for the \c PreconditionNotMetException creates
		 * an exception message describing the precondition violation using the
		 * plain-text description of the precondition provided as a parameter.
		 *
		 * \param precondDesc Plain-text description of the precondition that
		 *                    was not met.
		 */
		explicit PreconditionNotMetException(std::string const& precondDesc);

		/**
		 * \brief Copy the Exception Message from another \c PreconditionNotMetException Instance onto This One
		 *
		 * The copy constructor of the \c PreconditionNotMetException makes
		 * a deep copy of the exception message contained in another object
		 * instance onto this instance.
		 *
		 * \param other \c PreconditionNotMetException instance that will serve
		 *              as the source of the copy operation.
		 */
		inline PreconditionNotMetException(PreconditionNotMetException const& other)
		: m_exceptMsg(other.m_exceptMsg)
		{}
		/**
		 * \brief Tie in to The Destructor Chain of the \c std::exception Class
		 */
		virtual ~PreconditionNotMetException() throw();

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
#endif // !defined(EA_3DA7E627_BC2E_413c_8603_D1B27644B44B__INCLUDED_)
