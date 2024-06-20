/**
 * \file ThroughputCap.cpp
 * \brief Contains the implementation of the \c DdsKit::ThroughputCap class.
 * \date 2019-08-05 09:20:11
 * \author Rolando J. Nieves
 */

#include <cmath>
#include <limits>

#include <CoreKit/CoreKit.h>

#include "ThroughputCap.h"

using CoreKit::SystemTime;

namespace DdsKit
{

ThroughputCap::ThroughputCap(unsigned cap):
    m_cap(cap),
    m_lastSentTime(std::numeric_limits< double >::quiet_NaN()),
    m_sentCount(0u)
{

}


ThroughputCap::~ThroughputCap()
{

}


bool
ThroughputCap::canSend()
{
    bool result = false;

    if
    (
        !std::isnan(m_lastSentTime) &&
        (SystemTime::now() - m_lastSentTime < 1.0)
    )
    {
        result = m_sentCount < m_cap;
    }
    else
    {
        m_lastSentTime = SystemTime::now();
        m_sentCount = 0u;
        result = true;
    }

    if (result)
    {
        m_sentCount++;
    }

    return result;
}

} // end namespace DdsKit

// vim: set ts=4 sw=4 expandtab:
