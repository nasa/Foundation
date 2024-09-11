/**
 * \file IsoDdsKit/internal/BasicRandomIdSource.hpp
 * \brief Contains the definition of the \c IsoDdsKit::BasicRandomIdSource class.
 * \author Rolando J. Nieves
 * \date 2024-08-14 15:25:42
 */

#ifndef _FOUNDATION_ISODDSKIT_INTERNAL_BASICRANDOMIDSOURCE_HPP_
#define _FOUNDATION_ISODDSKIT_INTERNAL_BASICRANDOMIDSOURCE_HPP_

#include <random>

#include <IsoDdsKit/UniqueIdSource.hpp>


namespace IsoDdsKit
{

/**
 * \brief Service that attempts to generate unique identifiers using a random
 *        number generator.
 *
 * The identifiers yielded by this service use a random number generator with
 * replacement that spans almost all the unsigned 16-bit range \c [1,65535] .
 * As such, repeats are possible.
 */
class BasicRandomIdSource : public UniqueIdSource
{
private:
    using Engine = std::default_random_engine;
    using Distribution = std::uniform_int_distribution< int >;

    Engine m_engine;
    Distribution m_distribution;

public:
    /**
     * \brief Initialize the random number generator and distribution
     *        algorithm.
     *
     * This constructor selects a seed value at random.
     */
    BasicRandomIdSource();

    /**
     * \brief Initialize the random number generator and distribution
     *        algorithm using the provided seed value.
     *
     * \param seed Seed value to use in the random number generator.
     */
    BasicRandomIdSource(unsigned int seed);

    /**
     * \brief Copy construction not allowed.
     */
    BasicRandomIdSource(BasicRandomIdSource const& other) = delete;

    /**
     * \brief Move construction not allowed.
     */
    BasicRandomIdSource(BasicRandomIdSource&& other) = delete;

    virtual ~BasicRandomIdSource() = default;

    /**
     * \brief Generate random number in the range \c [1,65535]
     *
     * The random numbers are generated using
     * \c std::uniform_int_distribution<>
     *
     * \return The random number.
     */
    virtual int next() override;

    /**
     * \brief Copy assignment not allowed.
     */
    BasicRandomIdSource& operator=(BasicRandomIdSource const& other) = delete;

    /**
     * \brief Move assignment not allowed.
     */
    BasicRandomIdSource& operator=(BasicRandomIdSource&& other) = delete;
};

} // end namespace IsoDdsKit

#endif /* !_FOUNDATION_ISODDSKIT_INTERNAL_BASICRANDOMIDSOURCE_HPP_ */

// vim: set ts=4 sw=4 expandtab:
