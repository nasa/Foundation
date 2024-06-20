/**
 * \file string_utils.cpp
 * \brief Contains the implementation of the multi-vendor string utilities module.
 * \date 2016-06-17 13:55:11
 * \author Rolando J. Nieves
 */

#include <cstring>

#include "string_utils.h"

namespace DdsKit
{

#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
void SetUnboundedString(char *& stringField, const char *stringValue)
{
    stringField = strdup(stringValue);
}

void SetBoundedString(char *const stringField, const char *stringValue, unsigned maxSize)
{
    memset(stringField, 0x00, maxSize + 1);
    strncpy(stringField, stringValue, maxSize);
}
#endif /* DDS_PROVIDER */

}