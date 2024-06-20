/**
 * \file PreconditionNotMetException.cpp
 * \brief Implementation of the PreconditionNotMetException Class
 * \date 2012-09-24 14:15:11
 * \author Rolando J. Nieves
 */

#include <cstdlib>
#include <sstream>

#include "PreconditionNotMetException.h"

using std::string;
using std::stringstream;
using std::ends;
using CoreKit::PreconditionNotMetException;

PreconditionNotMetException::PreconditionNotMetException(string const& precondDesc)
{
	stringstream formattedMsg;

	formattedMsg << "Precondition Not Met:" << precondDesc << ends;
	m_exceptMsg = formattedMsg.str();
}

PreconditionNotMetException::~PreconditionNotMetException() throw()
{

}





const char* PreconditionNotMetException::what() const throw()
{
	return  m_exceptMsg.c_str();
}
