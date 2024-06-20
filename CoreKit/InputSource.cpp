/**
 * \file InputSource.cpp
 * \brief Implementation of the InputSource Class
 * \date 2012-09-24 08:39:57
 * \author Rolando J. Nieves
 */

#include <cstdlib>

#include "InputSource.h"
#include "InterruptListener.h"

using CoreKit::InputSource;
using CoreKit::InterruptListener;

uint8_t InputSource::NextDefaultPriority = 128u;

InputSource::InputSource()
: m_relativePriority(InputSource::NextDefaultPriority)
{
	if (InputSource::NextDefaultPriority < 255u)
	{
		InputSource::NextDefaultPriority++;
	}
}


InputSource::InputSource(uint8_t relativePriority)
: m_relativePriority(relativePriority)
{

}


InputSource::~InputSource()
{

}


int InputSource::fileDescriptor() const
{

	return 0;
}


InterruptListener* InputSource::interruptListener() const
{

	return  NULL;
}

void InputSource::fireCallback()
{

}
