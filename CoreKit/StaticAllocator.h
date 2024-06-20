/**
 * \file StaticAllocator.h
 * \brief Template definition of the \c StaticAllocator class.
 * \date 2012-03-08 11:20:00
 * \author Rolando J. Nieves
 */

#ifndef _FOUNDATION_COREKIT_STATICALLOCATOR_H_
#define _FOUNDATION_COREKIT_STATICALLOCATOR_H_

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <exception>

namespace CoreKit
{

/**
 * \brief Statically-allocated backing store C++ collection allocator.
 *
 * The \c CoreKit::StaticAllocator class implements a memory allocator compatible
 * with C++ collection classes. Unlike the standard allocators provided by the
 * C++ library, this allocator enforces a maximum size for the collection, and
 * based on that maximum size pre-allocates the memory it will need in one shot.
 * Doing so enables it to support a dynamically-sized collection that does not
 * need to re-allocate memory several times during its lifetime. The memory is
 * allocated as part of the allocator's object space, and the maximum size is
 * provided as a template argument.
 */
template<typename ValueType, int AllocSize>
class StaticAllocator
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
	struct rebind { typedef StaticAllocator<ValueType1, AllocSize> other; };

    /**
     * \brief Allocate backing store internally.
     *
     * The backing store is a pair of "ping-pong" arrays declared as fields of
     * this class.
     */
	StaticAllocator();

    /**
     * \brief Copy constructor.
     *
     * This constructor performs a deep copy of the content in the source
     * allocator.
     *
     * \param[in] other - Allocator instance that serves as the source of the
     *            copy operation.
     */
	StaticAllocator(StaticAllocator const& other);

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
     * \brief Switch between backing store "ping-pong" buffers.
     *
     * \param[in] allocSize - Size of requested backing store, expressed in
     *            element units. Only used to verify that the requested size
     *            does not go beyond the maximum size configured via the
     *            template parameters.
     *
     * \return Pointer to the allocated buffer.
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
     * \brief Access the maximum size configured via template parameters.
     *
     * \return Maximum backing store size, expressed in number of elements,
     *         configured via template parameters.
     */
	inline size_t max_size() const { return AllocSize; }

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
	inline bool operator ==(StaticAllocator<ValueType, AllocSize> const& rhs) const
	{ return this == &rhs; }
	
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
	inline bool operator !=(StaticAllocator<ValueType, AllocSize> const& rhs) const
	{ return this != &rhs; }
	
private:
    uint8_t m_staticBlock[2][AllocSize * sizeof(ValueType)];
	int m_nextBlock;
};


template<typename ValueType, int AllocSize>
StaticAllocator<ValueType, AllocSize>::StaticAllocator()
: m_nextBlock(0)
{
	memset(m_staticBlock, 0x00, sizeof(m_staticBlock));
}


template<typename ValueType, int AllocSize>
StaticAllocator<ValueType, AllocSize>::StaticAllocator(StaticAllocator const& other)
: m_nextBlock(other.m_nextBlock)
{
	memcpy(m_staticBlock, other.m_staticBlock, sizeof(m_staticBlock));
}


template<typename ValueType, int AllocSize>
void StaticAllocator<ValueType, AllocSize>::construct(ValueType *destPtr, ValueType const& copyRef)
{
    ::new((void*)destPtr) ValueType(copyRef);
}


template<typename ValueType, int AllocSize>
void StaticAllocator<ValueType, AllocSize>::destroy(ValueType *destPtr)
{
    destPtr->~ValueType();
}


template<typename ValueType, int AllocSize>
ValueType* StaticAllocator<ValueType, AllocSize>::allocate(size_t allocSize)
{
	ValueType *result = nullptr;

	if (allocSize > AllocSize)
	{
		throw std::bad_alloc();
	}

	result = m_staticBlock[m_nextBlock];
	m_nextBlock++;
	m_nextBlock %= 2;

	return result;
}


template<typename ValueType, int AllocSize>
void StaticAllocator<ValueType, AllocSize>::deallocate(ValueType *block, size_t allocSize)
{
	if ((block != m_staticBlock[0]) && (block != m_staticBlock[1]))
	{
		throw std::bad_alloc();
	}
}

template<typename ValueType, int AllocSize>
bool operator == (const StaticAllocator< ValueType, AllocSize >& left, const StaticAllocator< ValueType, AllocSize >& right)
{
    return left.max_size() == right.max_size();
}

}

#endif /* _FOUNDATION_COREKIT_STATICALLOCATOR_H_ */

// vim: set ts=4 sw=4 expandtab:
