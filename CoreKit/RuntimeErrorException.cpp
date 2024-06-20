/**
 * \file RuntimeErrorException.cpp
 * \brief Implementation of the RuntimeErrorException Class
 * \date 2012-09-24 16:49:03
 * \author Rolando J. Nieves
 */

#include <sstream>

#include "RuntimeErrorException.h"

using std::stringstream;
using std::ends;
using std::string;
using CoreKit::RuntimeErrorException;


RuntimeErrorException::RuntimeErrorException(string const& errorDescription)
{
	stringstream formattedMsg;

	formattedMsg << "Runtime Error: " << errorDescription << ends;
	m_exceptMsg = formattedMsg.str();
}

RuntimeErrorException::~RuntimeErrorException() throw()
{

}





const char* RuntimeErrorException::what() const throw()
{
	return m_exceptMsg.c_str();
}
