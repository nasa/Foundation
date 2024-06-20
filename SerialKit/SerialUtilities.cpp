/**
 * \file SerialUtilities.cpp
 * \brief Definition of Serial Utilities functions
 * \date Jan 3, 2017
 * \author Ryan O'Farrell
 * \copyright National Aeronautics and Space Administration
 */

//
// System Includes
//
#include <stdlib.h>

//
// Library Includes
//

//
// Project Includes
//
#include "SerialUtilities.h"


namespace SerialKit
{

speed_t StringToBaudRate(std::string const & baudRateString)
{
    char *endptr;
    long baudInt = strtol(baudRateString.c_str(), &endptr, 10);

    if (endptr == baudRateString.c_str() || *endptr != '\0')
    {
        return INVALID_BAUD_RATE;
    }
    else
    {
        return IntegerToBaudRate(baudInt);
    }
}

speed_t IntegerToBaudRate(int baudRateInt)
{
    speed_t baudRate = INVALID_BAUD_RATE;

    switch (baudRateInt)
    {
    case (0):
        baudRate = B0;
        break;
    case (50):
        baudRate = B50;
        break;
    case (75):
        baudRate = B75;
        break;
    case (110):
        baudRate = B110;
        break;
    case (134):
        baudRate = B134;
        break;
    case (150):
        baudRate = B150;
        break;
    case (200):
        baudRate = B200;
        break;
    case (300):
        baudRate = B300;
        break;
    case (600):
        baudRate = B600;
        break;
    case (1200):
        baudRate = B1200;
        break;
    case (1800):
        baudRate = B1800;
        break;
    case (2400):
        baudRate = B2400;
        break;
    case (4800):
        baudRate = B4800;
        break;
    case (9600):
        baudRate = B9600;
        break;
    case (19200):
        baudRate = B19200;
        break;
    case (38400):
        baudRate = B38400;
        break;
    case (57600):
        baudRate = B57600;
        break;
    case (115200):
        baudRate = B115200;
        break;
    case (230400):
        baudRate = B230400;
        break;
    case (460800):
        baudRate = B460800;
        break;
    case (500000):
        baudRate = B500000;
        break;
    case (576000):
        baudRate = B576000;
        break;
    case (921600):
        baudRate = B921600;
        break;
    case (1000000):
        baudRate = B1000000;
        break;
    case (1152000):
        baudRate = B1152000;
        break;
    case (2000000):
        baudRate = B2000000;
        break;
    case (2500000):
        baudRate = B2500000;
        break;
    case (3000000):
        baudRate = B3000000;
        break;
    case (3500000):
        baudRate = B3500000;
        break;
    case (4000000):
        baudRate = B4000000;
        break;
    }

    return baudRate;
}

} /* namespace SerialKit */
