/**
 * \file factory.h
 * \brief Contains the definitions for the CoreKit factory template functions.
 * \date 2013-03-21 10:37:00
 * \author Rolando J. Nieves
 */

#ifndef FACTORY_H_
#define FACTORY_H_

#define RF_CK_FACTORY_COMPATIBLE(className) public: inline static className* myType() { return (className*)0; }

namespace CoreKit
{

/**
 * \brief Create object instance taking no arguments.
 *
 * \param[in] typeTag Pointer argument used solely to acquire the object type.
 *
 * \return Newly-allocated object instance.
 */
template<typename ConstructType>
ConstructType* construct(ConstructType *typeTag)
{
	return new ConstructType();
}

/**
 * \brief Create object instance taking 1 argument.
 *
 * \param[in] typeTag Pointer argument used solely to acquire the object type.
 * \param[in] arg1 Argument to pass to constructor.
 *
 * \return Newly-allocated object instance.
 */
template<typename ConstructType, typename Arg1Type>
ConstructType* construct(ConstructType *typeTag, Arg1Type arg1)
{
	return new ConstructType(arg1);
}

/**
 * \brief Create object instance taking 2 arguments.
 *
 * \param[in] typeTag Pointer argument used solely to acquire the object type.
 * \param[in] arg1 Argument to pass to constructor.
 * \param[in] arg2 Argument to pass to constructor.
 *
 * \return Newly-allocated object instance.
 */
template<typename ConstructType,
         typename Arg1Type,
         typename Arg2Type>
ConstructType* construct(ConstructType *typeTag, Arg1Type arg1, Arg2Type arg2)
{
	return new ConstructType(arg1, arg2);
}

/**
 * \brief Create object instance taking 3 arguments.
 *
 * \param[in] typeTag Pointer argument used solely to acquire the object type.
 * \param[in] arg1 Argument to pass to constructor.
 * \param[in] arg2 Argument to pass to constructor.
 * \param[in] arg3 Argument to pass to constructor.
 *
 * \return Newly-allocated object instance.
 */
template<typename ConstructType,
         typename Arg1Type,
         typename Arg2Type,
         typename Arg3Type>
ConstructType* construct(ConstructType *typeTag, Arg1Type arg1, Arg2Type arg2, Arg3Type arg3)
{
	return new ConstructType(arg1, arg2, arg3);
}

/**
 * \brief Create object instance taking 4 arguments.
 *
 * \param[in] typeTag Pointer argument used solely to acquire the object type.
 * \param[in] arg1 Argument to pass to constructor.
 * \param[in] arg2 Argument to pass to constructor.
 * \param[in] arg3 Argument to pass to constructor.
 * \param[in] arg4 Argument to pass to constructor.
 *
 * \return Newly-allocated object instance.
 */
template<typename ConstructType,
         typename Arg1Type,
         typename Arg2Type,
         typename Arg3Type,
         typename Arg4Type>
ConstructType* construct(ConstructType *typeTag, Arg1Type arg1, Arg2Type arg2, Arg3Type arg3, Arg4Type arg4)
{
	return new ConstructType(arg1, arg2, arg3, arg4);
}

/**
 * \brief Create object instance taking 5 arguments.
 *
 * \param[in] typeTag Pointer argument used solely to acquire the object type.
 * \param[in] arg1 Argument to pass to constructor.
 * \param[in] arg2 Argument to pass to constructor.
 * \param[in] arg3 Argument to pass to constructor.
 * \param[in] arg4 Argument to pass to constructor.
 * \param[in] arg5 Argument to pass to constructor.
 *
 * \return Newly-allocated object instance.
 */
template<typename ConstructType,
         typename Arg1Type,
         typename Arg2Type,
         typename Arg3Type,
         typename Arg4Type,
         typename Arg5Type>
ConstructType* construct(ConstructType *typeTag, Arg1Type arg1, Arg2Type arg2, Arg3Type arg3, Arg4Type arg4, Arg5Type arg5)
{
	return new ConstructType(arg1, arg2, arg3, arg4, arg5);
}

/**
 * \brief Create object instance taking 6 arguments.
 *
 * \param[in] typeTag Pointer argument used solely to acquire the object type.
 * \param[in] arg1 Argument to pass to constructor.
 * \param[in] arg2 Argument to pass to constructor.
 * \param[in] arg3 Argument to pass to constructor.
 * \param[in] arg4 Argument to pass to constructor.
 * \param[in] arg5 Argument to pass to constructor.
 * \param[in] arg6 Argument to pass to constructor.
 *
 * \return Newly-allocated object instance.
 */
template<typename ConstructType,
         typename Arg1Type,
         typename Arg2Type,
         typename Arg3Type,
         typename Arg4Type,
         typename Arg5Type,
         typename Arg6Type>
ConstructType* construct(ConstructType *typeTag, Arg1Type arg1, Arg2Type arg2, Arg3Type arg3, Arg4Type arg4, Arg5Type arg5, Arg6Type arg6)
{
	return new ConstructType(arg1, arg2, arg3, arg4, arg5, arg6);
}

/**
 * \brief Create object instance taking 7 arguments.
 *
 * \param[in] typeTag Pointer argument used solely to acquire the object type.
 * \param[in] arg1 Argument to pass to constructor.
 * \param[in] arg2 Argument to pass to constructor.
 * \param[in] arg3 Argument to pass to constructor.
 * \param[in] arg4 Argument to pass to constructor.
 * \param[in] arg5 Argument to pass to constructor.
 * \param[in] arg6 Argument to pass to constructor.
 * \param[in] arg7 Argument to pass to constructor.
 *
 * \return Newly-allocated object instance.
 */
template<typename ConstructType,
         typename Arg1Type,
         typename Arg2Type,
         typename Arg3Type,
         typename Arg4Type,
         typename Arg5Type,
         typename Arg6Type,
         typename Arg7Type>
ConstructType* construct(ConstructType *typeTag, Arg1Type arg1, Arg2Type arg2, Arg3Type arg3, Arg4Type arg4, Arg5Type arg5, Arg6Type arg6, Arg7Type arg7)
{
	return new ConstructType(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
}

/**
 * \brief Destroy object instance.
 *
 * \param[in] theObj Pointer to the object instance to destroy.
 */
template<typename DestructType>
void destroy(DestructType *theObj)
{
	delete theObj;
}

/**
 * \brief Container-friendly callable used to destroy an object instance.
 */
template< typename DestructType >
struct DestroyCall
{
    /**
     * \brief Destroy object instance.
     *
     * \param[in] theObj Pointer to the object instance to destroy.
     */
    void operator() (DestructType *theObj) const
    { destroy(theObj); }
};

}

#endif /* FACTORY_H_ */
