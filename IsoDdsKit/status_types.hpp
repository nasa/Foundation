/**
 * \file IsoDdsKit/status_types.hpp
 * \brief Contains various type aliases for use within \c IsoDdsKit
 * \author Rolando J. Nieves
 * \date 2024-08-23 15:47:45
 */

#ifndef _FOUNDATION_ISODDSKIT_STATUS_TYPES_HPP_
#define _FOUNDATION_ISODDSKIT_STATUS_TYPES_HPP_

#include <dds/core/status/Status.hpp>

#include <IsoDdsKit/EmitStatusAction.hpp>
#include <IsoDdsKit/ValueDistribution.hpp>


namespace IsoDdsKit
{

// --------------------------------------------------------------------------
// For PublicationMatchedStatus
using PublicationMatchedStatusDistribution = ValueDistribution< dds::core::status::PublicationMatchedStatus >;
using EmitPublicationMatchedStatusAction = EmitStatusAction< dds::core::status::PublicationMatchedStatus >;

// --------------------------------------------------------------------------
// For SampleLostStatus
using SampleLostStatusDistribution = ValueDistribution< dds::core::status::SampleLostStatus >;
using EmitSampleLostStatusAction = EmitStatusAction< dds::core::status::SampleLostStatus >;

// --------------------------------------------------------------------------
// For SubscriptionMatchedStatus
using SubscriptionMatchedStatusDistribution = ValueDistribution< dds::core::status::SubscriptionMatchedStatus >;
using EmitSubscriptionMatchedStatusAction = EmitStatusAction< dds::core::status::SubscriptionMatchedStatus >;

} // end namespace IsoDdsKit

#endif /* !_FOUNDATION_ISODDSKIT_STATUS_TYPES_HPP_ */

// vim: set ts=4 sw=4 expandtab:
