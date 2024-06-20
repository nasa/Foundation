/**
 * \file FixedAllocator.h
 * \brief Template definition of the \c FixedAllocator class.
 * \date 2013-03-08 11:20:00
 * \author Rolando J. Nieves
 */

#ifndef _FOUNDATION_COREKIT_FIXEDALLOCATOR_H_
#define _FOUNDATION_COREKIT_FIXEDALLOCATOR_H_

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <exception>

namespace CoreKit
{

/**
 * \brief Fixed backing store C++ collection allocator.
 *
 * The \c CoreKit::FixedAllocator class implements a memory allocator compatible
 * with C++ collection classes. Unlike the standard allocators provided by the
 * C++ library, this allocator enforces a maximum size for the collection, and
 * based on that maximum size pre-allocates the memory it will need in one shot.
 * Doing so enables it to support a dynamically-sized collection that does not
 * need to re-allocate memory several times during its lifetime. The memory is
 * allocated off the heap.
 */
template<typename ValueType>
class FixedAllocator
{
public:
	typedef ValueType value_type;
	typedef ValueType* pointer;
	typedef ValueType const* const_pointer;
	typedef ValueType& reference;
	typedef ValueType const& const_reference;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;
	template<typename ValueType1>
	struct rebind { typedef FixedAllocator<ValueType1> other; };

    /**
     * \brief Initialize the instance field to nominal values.
     *
     * \param[in] maxSize - Maximum number of units the collection will hold.
     */
	explicit FixedAllocator(size_t maxSize);

    /**
     * \brief Copy constructor.
     *
     * \param[in] other - Reference to the object that serves as the source of
     *            the copy operation.
     */
	FixedAllocator(FixedAllocator const& other);

    /**
     * \brief Deallocate collection memory.
     */
	virtual ~FixedAllocator();

    /**
     * \brief Add new element to the collection backing store.
     *
     * Leverages the copy constructor to copy value contents into the
     * collection backing store.
     *
     * \param[in] destPtr - Backing store memory area where the new value will
     *            be stored.
     * \param[in] copyRef - Reference to the new value that will be stored.
     */
	void construct(ValueType* destPtr, ValueType const& copyRef);

    /**
     * \brief Remove element from the collection backing store.
     *
     * As of this writing this allocator does nothing in response to this
     * method call.
     *
     * \param[in] destPtr - Backing store memory area that holds the value to be
     *            removed.
     */
	void destroy(ValueType *destPtr);

    /**
     * \brief Pre-allocate maximum required memory upon first call.
     *
     * Normally this method would allocate enough space to hold a set number
     * of elements. In this class, the method allocates a pair of "ping-pong"
     * buffers capable of holding the maximum number of elements configured at
     * construction time.
     *
     * Once the buffers are allocated, they remain for the lifetime of the
     * allocator instance, thus making heap memory allocation far more
     * predictable than with standard allocators.
     *
     * \param[in] allocSize - Space requested expressed in number of collection
     *            elements. Only used to verify the request does not exceed the
     *            maximum space configured at construction time.
     */
	ValueType* allocate(size_t allocSize);

    /**
     * \brief No operation on this allocator.
     *
     * Normally, this method is used by C++ collections to release memory
     * allocated via the \c allocate() method. In this class, however, since
     * the allocated "ping-pong" backing stores remain available for the
     * lifetime of the allocator, this method does nothing.
     *
     * \param[in] block - Pointer to the beginning of the block created via
     *            \c allocate() . Unused.
     * \param[in] allocSize - Size of the block originally requested via
     *            \c allocate() . Unused.
     */
	void deallocate(ValueType *block, size_t allocSize);

    /**
     * \brief Access the maximum size configured at construction time.
     *
     * \return Maximum backing store size, expressed in number of elements,
     *         configured at construction time.
     */
	inline size_t max_size() const { return m_maxSize; }

    /**
     * \brief Determine shallow equality between allocators.
     *
     * This comparison is not a deep one, but rather one that determines
     * whether \c this allocator and the allocator provided as input use the
     * same backing store memory.
     *
     * \param[in] rhs - Allocator instance used in the comparison.
     *
     * \return \c true if the \c rhs allocator uses the same backing store as
     *         \c this allocator; \c false otherwise.
     */
	inline bool operator ==(FixedAllocator<ValueType> const& rhs) const
	{ return (this->m_allocBlock[0] == rhs.m_allocBlock[0]) && (this->m_allocBlock[1] == rhs.m_allocBlock[1]); }
	
    /**
     * \brief Determine inequality between allocators.
     *
     * This comparison is just a negation of the equality comparison result.
     *
     * \param[in] rhs - Allocator instance used in the comparison.
     *
     * \return \c true if the \c rhs allocator does not use the same backing
     *         stores as \c this allocator; \c false otherwise.
     */
	inline bool operator !=(FixedAllocator<ValueType> const& rhs) const
	{ return !(*this == rhs); }
	
private:
	ValueType *m_allocBlock[2];
	size_t m_maxSize;
	int m_nextBlock;
};

template<typename ValueType>
FixedAllocator<ValueType>::FixedAllocator(size_t maxSize)
: m_maxSize(maxSize), m_nextBlock(0)
{
	m_allocBlock[0] = nullptr;
	m_allocBlock[1] = nullptr;
}


template<typename ValueType>
FixedAllocator<ValueType>::FixedAllocator(FixedAllocator<ValueType> const& other)
: m_maxSize(other.m_maxSize), m_nextBlock(0)
{
	m_allocBlock[0] = nullptr;
	m_allocBlock[1] = nullptr;
}


template<typename ValueType>
FixedAllocator<ValueType>::~FixedAllocator()
{
	free(m_allocBlock[0]);
	free(m_allocBlock[1]);
}


template<typename ValueType>
void FixedAllocator<ValueType>::construct(ValueType *destPtr, ValueType const& copyRef)
{
    ::new((void*)destPtr) ValueType(copyRef);
}


template<typename ValueType>
void FixedAllocator<ValueType>::destroy(ValueType *destPtr)
{
    destPtr->~ValueType();
}


template<typename ValueType>
ValueType* FixedAllocator<ValueType>::allocate(size_t allocSize)
{
	ValueType *result = nullptr;

	if (allocSize > m_maxSize)
	{
		throw std::bad_alloc();
	}

	if (nullptr == m_allocBlock[m_nextBlock])
	{
		m_allocBlock[m_nextBlock] = (ValueType *)calloc(m_maxSize, sizeof(ValueType));
		if (nullptr == m_allocBlock[m_nextBlock])
		{
			throw std::bad_alloc();
		}
	}

	result = m_allocBlock[m_nextBlock];
	m_nextBlock++;
	m_nextBlock %= 2;

	return result;
}


template<typename ValueType>
void FixedAllocator<ValueType>::deallocate(ValueType *block, size_t allocSize)
{
	if ((block != m_allocBlock[0]) && (block != m_allocBlock[1]))
	{
		throw std::bad_alloc();
	}
}

}

#endif /* _FOUNDATION_COREKIT_FIXEDALLOCATOR_H_ */
