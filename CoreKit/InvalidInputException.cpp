/**
 * \file InvalidInputException.cpp
 * \brief Implementation of the InvalidInputException Class
 * \date 2012-09-24 16:48:45
 * \author Rolando J. Nieves
 */

#include <sstream>

#include "InvalidInputException.h"

using std::string;
using std::stringstream;
using std::ends;
using CoreKit::InvalidInputException;


InvalidInputException::InvalidInputException(std::string const& inputDescription, std::string const& inputValue)
{
	stringstream formattedMsg;

	formattedMsg << "Invalid input on " << inputDescription << ": Received \"" << inputValue << "\"" << ends;
	m_exceptMsg = formattedMsg.str();
}

InvalidInputException::~InvalidInputException() throw()
{

}





const char* InvalidInputException::what() const throw()
{
	return m_exceptMsg.c_str();
}
