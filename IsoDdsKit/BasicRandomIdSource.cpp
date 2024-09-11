/**
 * \file IsoDdsKit/internal/BasicRandomIdSource.cpp
 * \brief Contains the implementation for the \c IsoDdsKit::BasicRandomIdSource class.
 * \author Rolando J. Nieves
 * \date 2024-08-14 15:35:01
 */

#include "BasicRandomIdSource.hpp"


namespace
{

std::random_device DEFAULT_RANDOM_SEED_GEN;

constexpr int RANDOM_ID_MIN = 1;
constexpr int RANDOM_ID_MAX = 65535;

}

namespace IsoDdsKit
{

BasicRandomIdSource::BasicRandomIdSource():
    m_engine(DEFAULT_RANDOM_SEED_GEN()),
    m_distribution(RANDOM_ID_MIN, RANDOM_ID_MAX)
{
}

BasicRandomIdSource::BasicRandomIdSource(unsigned int seed):
    m_engine(seed),
    m_distribution(RANDOM_ID_MIN, RANDOM_ID_MAX)
{
}

int
BasicRandomIdSource::next()
{
    return m_distribution(m_engine);
}


} // end namespace IsoDdsKit

// vim: set ts=4 sw=4 expandtab:
