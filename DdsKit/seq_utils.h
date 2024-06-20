/**
 * \file seq_utils.h
 * \brief Contains convenience functions that wrap some sequence operations to alleviate multi-vendor differences
 * \date 2016-06-17 15:11:59
 * \author Rolando J. Nieves
 */

#ifndef _DDSKIT_SEQ_UTILS_H_
#define _DDSKIT_SEQ_UTILS_H_

#include <DdsKit/dds_provider.h>

namespace DdsKit
{

#if defined(DDS_PROVIDER)

// ============================================================================
// CoreDX
// ============================================================================

# if defined(DDS_PROVIDER_COREDX) && (DDS_PROVIDER == DDS_PROVIDER_COREDX)
/**
 * \brief Add value to a DDS sequence.
 *
 * This function is used to abstract away the differences between DDS providers
 * regarding sequence management.
 *
 * \param value Reference to value that will be added.
 * \param theSeq Reference to the sequence that will receive the new value.
 */
template< typename SequenceType, typename ValueType >
void AppendValueToSequence(ValueType const& value, SequenceType& theSeq)
{
    theSeq.push_back(value);
}

/**
 * \brief Remove a value from DDS sequence by index.
 *
 * This function is used to abstract away the differences between DDS providers
 * regarding sequence management.
 *
 * \note Not implemented as of this writing.
 *
 * \param theSeq Reference to sequence that will be edited.
 * \param index Index to the value that will be removed.
 */
template< typename SequenceType >
void RemoveFromSequenceAtIndex(SequenceType& theSeq, unsigned index)
{}

/**
 * \brief Determine the total length of a DDS sequence.
 *
 * This function is used to abstract away the differences between DDS providers
 * regarding sequence management.
 *
 * \param theSeq Reference to sequence that will be evaluated.
 *
 * \return Number of elements in DDS sequence.
 */
template< typename SequenceType >
inline unsigned SequenceLength(SequenceType const& theSeq)
{
    return theSeq.size();
}

/**
 * \brief Assign sequence of new elements to DDS sequence.
 *
 * This function is used to abstract away the differences between DDS providers
 * regarding sequence management.
 *
 * \note The memory pointed to by the \c array parameter is adopted by the
 * DDS sequence and must not be de-allocated by the caller.
 *
 * \param array Sequence of elements to add.
 * \param arrayLen Number of elements in \c array sequence.
 * \param theSeq Reference to sequence that will receive values.
 */
template< typename SequenceType, typename ElementType >
inline void AssignArrayToSequence(ElementType* array, size_t arrayLen, SequenceType& theSeq)
{
    theSeq.assign(array, arrayLen);
}
# endif /* (DDS_PROVIDER == DDS_PROVIDER_COREDX) */

// ============================================================================
// OpenSplice
// ============================================================================

# if defined(DDS_PROVIDER_OSPL) && (DDS_PROVIDER == DDS_PROVIDER_OSPL)
/**
 * \brief Add value to a DDS sequence.
 *
 * This function is used to abstract away the differences between DDS providers
 * regarding sequence management.
 *
 * \param value Reference to value that will be added.
 * \param theSeq Reference to the sequence that will receive the new value.
 */
template< typename SequenceType, typename ValueType >
void AppendValueToSequence(ValueType const& value, SequenceType& theSeq)
{
    // The way to grow OpenSplice sequences is to set the length to a value
    // greater than the current length. Unfortunately, OpenSplice chooses 
    // make this new length the maximum capacity, which for repeated 
    // append operations can become inefficient. Thus, we use the length()
    // operation to first grow the maximum length, then we trim it back
    // to what's necessary.
    if (theSeq.maximum() == 0)
    {
        theSeq.length(100u); // Start with a sequence maximum length of 100
        theSeq.length(0u);
    }
    else if (theSeq.maximum() <= (theSeq.length() + 1))
    {
        theSeq.length(theSeq.maximum() * 2); // Always double the maximum storage when we run out
        theSeq.length(theSeq.length());
    }
    unsigned newElemIdx = theSeq.length();
    theSeq.length(theSeq.length() + 1);
    theSeq[newElemIdx] = value;
}

/**
 * \brief Remove a value from DDS sequence by index.
 *
 * This function is used to abstract away the differences between DDS providers
 * regarding sequence management.
 *
 * \note Not implemented as of this writing.
 *
 * \param theSeq Reference to sequence that will be edited.
 * \param index Index to the value that will be removed.
 */
template< typename SequenceType >
void RemoveFromSequenceAtIndex(SequenceType& theSeq, unsigned index)
{}

/**
 * \brief Determine the total length of a DDS sequence.
 *
 * This function is used to abstract away the differences between DDS providers
 * regarding sequence management.
 *
 * \param theSeq Reference to sequence that will be evaluated.
 *
 * \return Number of elements in DDS sequence.
 */
template< typename SequenceType >
inline unsigned SequenceLength(SequenceType const& theSeq)
{
    return theSeq.length();
}

/**
 * \brief Assign sequence of new elements to DDS sequence.
 *
 * This function is used to abstract away the differences between DDS providers
 * regarding sequence management.
 *
 * \note The memory pointed to by the \c array parameter is adopted by the
 * DDS sequence and must not be de-allocated by the caller.
 *
 * \param array Sequence of elements to add.
 * \param arrayLen Number of elements in \c array sequence.
 * \param theSeq Reference to sequence that will receive values.
 */
template< typename SequenceType, typename ElementType >
inline void AssignArrayToSequence(ElementType* array, size_t arrayLen, SequenceType& theSeq)
{
    theSeq.replace(arrayLen, array, TRUE);
}
# endif /* (DDS_PROVIDER == DDS_PROVIDER_OSPL) */

// ============================================================================
// OpenDDS
// ============================================================================

# if defined(DDS_PROVIDER_ODDS) && (DDS_PROVIDER == DDS_PROVIDER_ODDS)
/**
 * \brief Add value to a DDS sequence.
 *
 * This function is used to abstract away the differences between DDS providers
 * regarding sequence management.
 *
 * \param value Reference to value that will be added.
 * \param theSeq Reference to the sequence that will receive the new value.
 */
template< typename SequenceType, typename ValueType >
void AppendValueToSequence(ValueType const& value, SequenceType& theSeq)
{
    // The way to grow OpenSplice sequences is to set the length to a value
    // greater than the current length. Unfortunately, OpenSplice chooses 
    // make this new length the maximum capacity, which for repeated 
    // append operations can become inefficient. Thus, we use the length()
    // operation to first grow the maximum length, then we trim it back
    // to what's necessary.
    if (theSeq.maximum() == 0)
    {
        theSeq.length(100u); // Start with a sequence maximum length of 100
        theSeq.length(0u);
    }
    else if (theSeq.maximum() <= (theSeq.length() + 1))
    {
        theSeq.length(theSeq.maximum() * 2); // Always double the maximum storage when we run out
        theSeq.length(theSeq.length());
    }
    unsigned newElemIdx = theSeq.length();
    theSeq.length(theSeq.length() + 1);
    theSeq[newElemIdx] = value;
}

/**
 * \brief Remove a value from DDS sequence by index.
 *
 * This function is used to abstract away the differences between DDS providers
 * regarding sequence management.
 *
 * \note Not implemented as of this writing.
 *
 * \param theSeq Reference to sequence that will be edited.
 * \param index Index to the value that will be removed.
 */
template< typename SequenceType >
void RemoveFromSequenceAtIndex(SequenceType& theSeq, unsigned index)
{}

/**
 * \brief Determine the total length of a DDS sequence.
 *
 * This function is used to abstract away the differences between DDS providers
 * regarding sequence management.
 *
 * \param theSeq Reference to sequence that will be evaluated.
 *
 * \return Number of elements in DDS sequence.
 */
template< typename SequenceType >
inline unsigned SequenceLength(SequenceType const& theSeq)
{
    return theSeq.length();
}

/**
 * \brief Assign sequence of new elements to DDS sequence.
 *
 * This function is used to abstract away the differences between DDS providers
 * regarding sequence management.
 *
 * \note The memory pointed to by the \c array parameter is adopted by the
 * DDS sequence and must not be de-allocated by the caller.
 *
 * \param array Sequence of elements to add.
 * \param arrayLen Number of elements in \c array sequence.
 * \param theSeq Reference to sequence that will receive values.
 */
template< typename SequenceType, typename ElementType >
inline void AssignArrayToSequence(ElementType* array, size_t arrayLen, SequenceType& theSeq)
{
    theSeq.replace(arrayLen, array, TRUE);
}
# endif /* (DDS_PROVIDER == DDS_PROVIDER_ODDS) */

#endif /* DDS_PROVIDER */

} // namespace DdsKit

#endif /* !_DDSKIT_SEQ_UTILS_H_ */
