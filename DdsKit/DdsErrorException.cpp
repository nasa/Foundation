/**
 * \file DdsErrorException.cpp
 * \brief Implementation of the DdsErrorException Class
 * \date 2012-09-27 14:24:38
 * \author Rolando J. Nieves
 */

#include "DdsErrorException.h"

using std::string;
using std::stringstream;
using std::ends;
using DDS::ReturnCode_t;

namespace DdsKit {

DdsErrorException::DdsErrorException(string const& description, DDS::ReturnCode_t errorCode)
: m_description(description), m_errorCode(errorCode)
{
#if defined(DDS_PROVIDER) && (DDS_PROVIDER == DDS_PROVIDER_OSPL)
	DDS::ErrorInfo errInfo;
	char *errLocation = NULL;
	char *errMessage = NULL;
	errInfo.update();
	if (errInfo.get_location(errLocation) == DDS::RETCODE_NO_DATA)
	{
		errLocation = strdup("<Unknown>");
	}
	if (errInfo.get_message(errMessage) == DDS::RETCODE_NO_DATA)
	{
		errMessage = strdup("<Unknown>");
	}
#endif /* DDS_PROVIDER == DDS_PROVIDER_OSPL */
	stringstream formattedMsg;

	formattedMsg << "DdsKit: "
		<< description
		<< ": "
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
		<< DDS_error(errorCode)
#elif defined(DDS_PROVIDER) && (DDS_PROVIDER == DDS_PROVIDER_OSPL)
		<< "At (" << errLocation << "): "
		<< errMessage
#endif /* DDS_PROVIDER */
		;
	m_exceptMsg = formattedMsg.str();
}


DdsErrorException::DdsErrorException(DdsErrorException const& other)
: m_description(other.m_description), m_errorCode(other.m_errorCode), m_exceptMsg(other.m_exceptMsg)
{

}


DdsErrorException::~DdsErrorException() throw()
{

}

char const* DdsErrorException::what() const throw()
{
	return m_exceptMsg.c_str();
}

} // namespace DdsKit
