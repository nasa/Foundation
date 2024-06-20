/**
 * \file ByteVector.h
 * \brief Contains the definition of the \c StaticByteVector, \c FixedByteVector, and \c DynamicByteVector template classes.
 * \date 2013-03-08 11:51:00
 * \author Rolando J. Nieves
 */

#ifndef _FOUNDATION_COREKIT_STATICBYTEVECTOR_H_
#define _FOUNDATION_COREKIT_STATICBYTEVECTOR_H_

#include <algorithm>
#include <cstdint>
#include <exception>
#include <vector>

#include <CoreKit/FixedAllocator.h>
#include <CoreKit/StaticAllocator.h>

namespace CoreKit
{

/**
 * \brief Stack-allocated byte array with vector-like semantics.
 *
 * The \c CoreKit::StaticAllocator template class does not allocate its memory
 * off the heap, but rather off the stack. The maximum size of the byte array is
 * part of the resulting class' type information.
 *
 * \deprecated This feature was added to Foundation before the C++11 standard
 *             was more widely adopted. As of this writing, C++11 standard
 *             library \c std::vector and \c std::array collections more than
 *             make up for the features offered by this class. Thus, this class
 *             will be removed in the next major release of Foundation.
 */
template<int AllocSize>
class StaticByteVector : public std::vector<uint8_t, StaticAllocator<uint8_t, AllocSize> >
{
public:
    /**
     * \brief Default constructor.
     *
     * Initialize base \c std::vector with the static allocator it should use
     * for a backing store.
     */
    StaticByteVector()
    : std::vector<uint8_t, StaticAllocator<uint8_t, AllocSize> >()
    { std::vector<uint8_t, StaticAllocator<uint8_t, AllocSize> >::reserve(AllocSize); }
};

/**
 * \brief Vector-like byte array with single heap-allocated backing store.
 *
 * The \c CoreKit::FixedAllocator template class allocates enough heap memory
 * at initialization time to hold the maximum size specified at construction
 * time. The maximum size of the byte array is \b not part of the resulting
 * class' type information.
 * 
 * \deprecated This feature was added to Foundation before the C++11 standard
 *             was more widely adopted. As of this writing, C++11 standard
 *             library \c std::vector and \c std::array collections more than
 *             make up for the features offered by this class. Thus, this class
 *             will be removed in the next major release of Foundation.
 */
class FixedByteVector : public std::vector<uint8_t, FixedAllocator<uint8_t> >
{
public:
    explicit FixedByteVector(size_t maxSize)
    : std::vector<uint8_t, FixedAllocator<uint8_t> >(FixedAllocator<uint8_t>(maxSize))
    { reserve(maxSize); }
};

/**
 * \brief Type alias for a standard C++ \c std::vector that holds bytes.
 * 
 * \deprecated This feature was added to Foundation before the C++11 standard
 *             was more widely adopted. As of this writing, C++11 standard
 *             library \c std::vector and \c std::array collections more than
 *             make up for the features offered by this class. Thus, this class
 *             will be removed in the next major release of Foundation.
 */
class DynamicByteVector : public std::vector<uint8_t>
{
};


/**
 * \brief Decode bytes from array onto strictly-typed element.
 *
 * The size of \c Element determines how many bytes to decode from the byte
 * array. This function does \b not perform any byte swaps.
 *
 * \param[in] vectorPos - Iterator instance pointing to the first byte to
 *            decode.
 * \param[out] resultElem - Target of the decoded bytes.
 */
template<typename IteratorKind, typename Element>
IteratorKind BytesToValue(IteratorKind vectorPos, Element& resultElem)
{
    union ElemUnion {
        Element theElem;
        uint8_t elemBytes[sizeof(Element)];
    } *elemUnion = (ElemUnion *)&resultElem;
    IteratorKind startPos = vectorPos;
    IteratorKind endPos = (startPos + sizeof(Element));

    std::copy(startPos, endPos, &elemUnion->elemBytes[0]);

    return endPos;
}


/**
 * \brief Encode the bytes of a strictly-typed element into a byte array.
 *
 * The size of \c Element determines how many bytes to encode into the array.
 * This function does \b not perform any byte swaps. The function assumes there
 * is enough space to fit the encoded bytes, and does \b not grow the receiving
 * array.
 *
 * \param[in] anElem - Source of the bytes to encode.
 * \param[in] vectorPos - Iterator instance pointing to the first space where
 *            the bytes should be added.
 *
 * \return Iterator pointing to the byte in the array immediately after the
 *         inserted bytes.
 */
template<typename IteratorKind, typename Element>
IteratorKind ValueToBytes(Element const& anElem, IteratorKind vectorPos)
{
    union ElemUnion {
        Element theElem;
        uint8_t elemBytes[sizeof(Element)];
    } *elemUnion = (ElemUnion *)&anElem;

    std::copy(&elemUnion->elemBytes[0], &elemUnion->elemBytes[sizeof(Element)], vectorPos);

    return (vectorPos + sizeof(Element));
}

/**
 * \brief Add bytes to the end of a vector-like byte array.
 *
 * Unlike the \c CoreKit::ValueToBytes or \c CoreKit::BytesToValue functions
 * that don't alter the size of the byte array they operate on, this function
 * will grow the array with the bytes it encodes.
 *
 * \param[in] anElem - Source of the bytes to encode.
 * \param[in,out] containerRef - Reference to the byte array that will receive
 *                the encoded bytes.
 */
template<typename ContainerType, typename Element>
void AppendBytesToVector(Element const& anElem, ContainerType & containerRef)
{
    union ElemUnion {
        Element theElem;
        uint8_t elemBytes[sizeof(Element)];
    } *elemUnion = (ElemUnion *)&anElem;

    containerRef.insert(containerRef.end(), &elemUnion->elemBytes[0], &elemUnion->elemBytes[sizeof(Element)]);
}

} // end namespace CoreKit

#endif /* _FOUNDATION_COREKIT_STATICBYTEVECTOR_H_ */
