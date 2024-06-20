/**
 * \file SerialUtilities.h
 * \brief Declration of Serial Utilities functions
 * \date Jan 3, 2017
 * \author Ryan O'Farrell
 * \copyright National Aeronautics and Space Administration
 */

#ifndef SERIALKIT_SERIALUTILITIES_H_
#define SERIALKIT_SERIALUTILITIES_H_

//
// System Includes
//
#include <termios.h>
#include <unistd.h>
#include <string>

//
// Library Includes
//

//
// Project Includes
//

namespace SerialKit
{

const speed_t INVALID_BAUD_RATE = 0xFFFFFFFF;

/**
 * \brief Converts a string to a baud rate constant
 * \param baudRateString the baud rate as a string
 * \return the matching baud rate as speed_t, or 0xFFFFFFFF if invalid
 */
speed_t StringToBaudRate(std::string const & baudRateString);

/**
 * \brief Converts an int to a baud rate constant
 * \param baudRateInt the baud rate as an int
 * \return the matching baud rate as speed_t, or 0xFFFFFFFF if invalid
 */
speed_t IntegerToBaudRate(int baudRateInt);

} /* namespace SerialKit */

#endif /* SERIALKIT_SERIALUTILITIES_H_ */
