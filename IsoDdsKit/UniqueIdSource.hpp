/**
 * \file IsoDdsKit/UniqueIdSource.hpp
 * \brief Contains the definition of the \c IsoDdsKit::UniqueIdSource class.
 * \author Rolando J. Nieves
 * \date 2024-08-14 15:00:35
 */

#ifndef _FOUNDATION_ISODDSKIT_UNIQUEIDSOURCE_HPP_
#define _FOUNDATION_ISODDSKIT_UNIQUEIDSOURCE_HPP_

namespace IsoDdsKit
{

/**
 * \brief Interface for services that attempt to provide unique identifier values.
 *
 * Services implementing this interface may use any algorithm to generate
 * identifiers. This interface does, however, restrict these unique identifiers
 * to be machine-native integer values (i.e., values of \c int type).
 */
class UniqueIdSource
{
public:
    /**
     * \brief Generate a new identifier value.
     *
     * Pure virtual method meant for implementation by concrete services.
     *
     * \return New unique identifier value.
     */
    virtual int next() = 0;
};

} // end namespace IsoDdsKit

#endif /* !_FOUNDATION_ISODDSKIT_UNIQUEIDSOURCE_HPP_ */

// vim: set ts=4 sw=4 expandtab: