/**
 * \file DdsErrorException.h
 * \brief Definition of the DdsErrorException Class
 * \date 2012-09-27 14:24:38
 * \author Rolando J. Nieves
 */
#if !defined(EA_EE6577D9_62FA_475f_9AE4_3489F1A8426D__INCLUDED_)
#define EA_EE6577D9_62FA_475f_9AE4_3489F1A8426D__INCLUDED_

#include <exception>
#include <string>
#include <sstream>

#include <DdsKit/dds_provider.h>

namespace DdsKit
{
	/**
	 * \brief Exception class used to communicate DDS errors.
	 * \author Rolando J. Nieves
	 * \date 2012-09-27 14:21:55
	 */
	class DdsErrorException : public std::exception
	{

	public:
                /**
                 * \brief Constructor
                 * \param description the exception description
                 * \param errorCode the DDS error code
                 */
		DdsErrorException(std::string const& description, DDS::ReturnCode_t errorCode);

                /**
                 * \brief Copy constructor
                 * \param other the instance to copy
                 */
		DdsErrorException(DdsErrorException const& other);

                /**
                 * \brief Destructor
                 */
		virtual ~DdsErrorException() throw();

                /**
                 * \brief Get the reason for the exception
                 * \return the exception description
                 */
		virtual char const* what() const throw();

	private:
		const std::string m_description;
		const DDS::ReturnCode_t m_errorCode;
		std::string m_exceptMsg;
	};

}
#endif // !defined(EA_EE6577D9_62FA_475f_9AE4_3489F1A8426D__INCLUDED_)
