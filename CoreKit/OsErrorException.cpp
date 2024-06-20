/**
 * \file OsErrorException.cpp
 * \brief Implementation of the OsErrorException Class
 * \date 2012-09-24 14:42:03
 * \author Rolando J. Nieves
 */

#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <sstream>

#include "OsErrorException.h"

using std::string;
using std::stringstream;
using std::ends;
using CoreKit::OsErrorException;


OsErrorException::OsErrorException(string const& osCall, int errorNumber)
{
	stringstream formattedMsg;

	formattedMsg << "OS Error on \"" << osCall << "()\": " << strerror(errorNumber) << ends;
	m_exceptMsg = formattedMsg.str();
}

OsErrorException::~OsErrorException() throw()
{

}





const char* OsErrorException::what() const throw()
{
	return  m_exceptMsg.c_str();
}
