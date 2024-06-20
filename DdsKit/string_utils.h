/**
 * \file string_utils.h
 * \brief Contains convenience functions that wrap string operations in order to alleviate multi-vendor differences.
 * \date 2016-06-17 13:26:42
 * \author Rolando J. Nieves
 */

#ifndef _DDSKIT_STRING_UTILS_H_
#define _DDSKIT_STRING_UTILS_H_

#include <DdsKit/dds_provider.h>

namespace DdsKit
{

#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
/**
 * \brief Set the contents of an unbounded DDS string.
 *
 * This function is used to abstract away the differences between DDS providers
 * regarding string management.
 *
 * \param stringField DDS sample field to alter.
 * \param stringValue Content to initialize the DDS string with.
 */
void SetUnboundedString(char *& stringField, const char *stringValue);


/**
 * \brief Set the contents of a bounded DDS string.
 *
 * \note If the provided contents are longer than what the DDS string can hold,
 * the assignment operation will be truncated.
 *
 * This function is used to abstract away the differences between DDS providers
 * regarding string management.
 *
 * \param stringField DDS sample field to alter.
 * \param stringValue Cntent to initialize the DDS string with.
 * \param maxSize Maximum content size the DDS field can accept.
 */
void SetBoundedString(char *const stringField, const char *stringValue, unsigned maxSize);

/**
 * \brief Retrieve the contents of a DDS string field as a character array.
 *
 * This function is used to abstract away the differences between DDS providers
 * regarding string management.
 *
 * \param stringField DDS sample field to evaluate.
 *
 * \return DDS field contents as a character string.
 */
inline const char*
GetStringContents(const char* stringField) { return stringField; }

#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
/**
 * \brief Set the contents of an unbounded DDS string.
 *
 * This function is used to abstract away the differences between DDS providers
 * regarding string management.
 *
 * \param stringField DDS sample field to alter.
 * \param stringValue Content to initialize the DDS string with.
 */
template< typename StringManager >
void SetUnboundedString(StringManager& stringField, const char *stringValue)
{
    stringField = stringValue;
}

/**
 * \brief Set the contents of a bounded DDS string.
 *
 * This function is used to abstract away the differences between DDS providers
 * regarding string management.
 *
 * \param stringField DDS sample field to alter.
 * \param stringValue Cntent to initialize the DDS string with.
 * \param maxSize Maximum content size the DDS field can accept.
 */
template< typename StringManager >
void SetBoundedString(StringManager& stringField, const char *stringValue, unsigned maxSize)
{
    stringField = stringValue;
}

/**
 * \brief Retrieve the contents of a DDS string field as a character array.
 *
 * This function is used to abstract away the differences between DDS providers
 * regarding string management.
 *
 * \param stringField DDS sample field to evaluate.
 *
 * \return DDS field contents as a character string.
 */
template< typename StringManager >
const char*
GetStringContents(StringManager const& stringField)
{
    return (const char*)stringField;
}

#endif /* DDS_PROVIDER */

} // namespace DdsKit

#endif /* _DDSKIT_STRING_UTILS_H_ */