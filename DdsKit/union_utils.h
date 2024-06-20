/**
 * \file union_utils.h
 * \brief Contains convenience functions for dealing with DDS unions across multiple vendors.
 * \date 2018-08-02 10:56:53
 * \author Rolando J. Nieves
 */

#ifndef _FOUNDATION_DDSKIT_UNION_UTILS_H_
#define _FOUNDATION_DDSKIT_UNION_UTILS_H_

#include <DdsKit/dds_provider.h>

namespace DdsKit
{

#if defined(DDS_PROVIDER)
/**
 * \brief Retrieve the value discriminator associated with a DDS union field.
 *
 * This function is used to abstract away the differences between DDS providers
 * regarding union management.
 *
 * \param theUnion DDS union to be evaluated.
 *
 * \return Value discriminator associated with the union value.
 */
template< typename DiscriminatorEnum, typename UnionType >
inline DiscriminatorEnum
UnionDiscriminatorValue(UnionType const& theUnion)
{
# if defined(DDS_PROVIDER_COREDX) && (DDS_PROVIDER == DDS_PROVIDER_COREDX)
    return theUnion.discriminator();
# elif defined(DDS_PROVIDER_OSPL) && (DDS_PROVIDER == DDS_PROVIDER_OSPL)
    return theUnion._d();
# elif defined(DDS_PROVIDER_ODDS) && (DDS_PROVIDER == DDS_PROVIDER_ODDS)
    return theUnion._d();
# endif /* DDS_PROVIDER selection */
}
#endif /* defined(DDS_PROVIDER) */

} // end namespace DdsKit

#endif /* !_FOUNDATION_DDSKIT_UNION_UTILS_H_ */