/**
 * \file BoundMember.h
 * \brief Contains the class and function definitions that implement C++ Bound Member Functors
 * \date 2013-12-16 14:45:15
 * \author Rolando J. Nieves
 */

#ifndef _FOUNDATION_COREKIT_BOUNDMEMBER_H_
#define _FOUNDATION_COREKIT_BOUNDMEMBER_H_

namespace CoreKit
{

/**
 * \brief Template for instance-bound function-like method accepting one argument.
 *
 * \deprecated This feature was created prior to the wide acceptance of the
 *             C++11 standard and its much more capable \c std::bind() facility.
 *             As such, these calls and their corresponding factory functions
 *             will be removed in the next major release of Foundation.
 */
template<class ClassType, typename ReturnType, typename ArgumentType>
class BoundMemberFunction1
{
public:
    /**
     * \brief Initialize the instance fields.
     *
     * \note Instances of this class don't guard against the target object
     *       becoming invalid. That is the responsibility of the user.
     *
     * \param[in] targetObject - Pointer to the object that is the target of the
     *            method call.
     * \param[in] method - Pointer to the instance method that will be called.
     */
	BoundMemberFunction1(ClassType *targetObject, ReturnType (ClassType::*method)(ArgumentType))
	: m_targetObject(targetObject), m_method(method)
	{}

    /**
     * \brief Copy constructor.
     */
	BoundMemberFunction1(BoundMemberFunction1 const& other) = default;

    /**
     * \brief Destructor.
     */
	virtual ~BoundMemberFunction1() = default;

    /**
     * \brief Call the instance method on the target object.
     *
     * \param[in] argument Argument to pass to the method.
     *
     * \return Value returned by the target object method.
     */
	ReturnType operator()(ArgumentType argument)
	{ return (m_targetObject->*m_method)(argument); }

    /**
     * \brief Copy assignment operator.
     */
	BoundMemberFunction1& operator=(BoundMemberFunction1 const& other) = default;

private:
	ClassType *m_targetObject;
	ReturnType (ClassType::*m_method)(ArgumentType);

	BoundMemberFunction1();
};

/**
 * \brief Template for instance-bound procedure-like method accepting one argument.
 *
 * \note A distinction is drawn between procedure-like and function-like methods
 *       based on whether they return a value or not.
 *      
 * \deprecated This feature was created prior to the wide acceptance of the
 *             C++11 standard and its much more capable \c std::bind() facility.
 *             As such, these calls and their corresponding factory functions
 *             will be removed in the next major release of Foundation.
 */
template<class ClassType, typename ArgumentType>
class BoundMemberProcedure1
{
public:
    /**
     * \brief Initialize the instance fields.
     *
     * \note Instances of this class don't guard against the target object
     *       becoming invalid. That is the responsibility of the user.
     *
     * \param[in] targetObject - Pointer to the object that is the target of the
     *            method call.
     * \param[in] method - Pointer to the instance method that will be called.
     */
	BoundMemberProcedure1(ClassType *targetObject, void (ClassType::*method)(ArgumentType))
	: m_targetObject(targetObject), m_method(method)
	{}

    /**
     * \brief Copy constructor.
     */
	BoundMemberProcedure1(BoundMemberProcedure1 const& other) = default;

    /**
     * \brief Destructor.
     */
	virtual ~BoundMemberProcedure1() = default;

    /**
     * \brief Call the instance method on the target object.
     *
     * \param[in] argument Argument to pass to the method.
     */
	void operator()(ArgumentType argument)
	{ (m_targetObject->*m_method)(argument); }

    /**
     * \brief Copy assignment operator.
     */
	BoundMemberProcedure1& operator=(BoundMemberProcedure1 const& other) = default;

private:
	ClassType *m_targetObject;
	void (ClassType::*m_method)(ArgumentType);

	BoundMemberProcedure1();
};


/**
 * \brief Template for instance-bound function-like method accepting two arguments.
 *
 * \deprecated This feature was created prior to the wide acceptance of the
 *             C++11 standard and its much more capable \c std::bind() facility.
 *             As such, these calls and their corresponding factory functions
 *             will be removed in the next major release of Foundation.
 */
template<class ClassType, typename ReturnType, typename Argument1Type, typename Argument2Type>
class BoundMemberFunction2
{
public:
    /**
     * \brief Initialize the instance fields.
     *
     * \note Instances of this class don't guard against the target object
     *       becoming invalid. That is the responsibility of the user.
     *
     * \param[in] targetObject - Pointer to the object that is the target of the
     *            method call.
     * \param[in] method - Pointer to the instance method that will be called.
     */
	BoundMemberFunction2(ClassType *targetObject, ReturnType (ClassType::*method)(Argument1Type, Argument2Type))
	: m_targetObject(targetObject), m_method(method)
	{}

    /**
     * \brief Copy constructor.
     */
	BoundMemberFunction2(BoundMemberFunction2 const& other) = default;

    /**
     * \brief Destructor.
     */
	virtual ~BoundMemberFunction2() = default;

    /**
     * \brief Call the instance method on the target object.
     *
     * \param[in] argument1 First argument to pass to the method.
     * \param[in] argument2 Second argument to pass to the method.
     *
     * \return Value returned by the target object method.
     */
	ReturnType operator()(Argument1Type argument1, Argument2Type argument2)
	{ return (m_targetObject->*m_method)(argument1, argument2); }

    /**
     * \brief Copy assignment operator.
     */
	BoundMemberFunction2& operator=(BoundMemberFunction2 const& other) = default;

private:
	ClassType *m_targetObject;
	ReturnType (ClassType::*m_method)(Argument1Type, Argument2Type);

	BoundMemberFunction2();
};

/**
 * \brief Template for instance-bound procedure-like method accepting one argument.
 *
 * \note A distinction is drawn between procedure-like and function-like methods
 *       based on whether they return a value or not.
 *
 * \deprecated This feature was created prior to the wide acceptance of the
 *             C++11 standard and its much more capable \c std::bind() facility.
 *             As such, these calls and their corresponding factory functions
 *             will be removed in the next major release of Foundation.
 */
template<class ClassType, typename Argument1Type, typename Argument2Type>
class BoundMemberProcedure2
{
public:
    /**
     * \brief Initialize the instance fields.
     *
     * \note Instances of this class don't guard against the target object
     *       becoming invalid. That is the responsibility of the user.
     *
     * \param[in] targetObject - Pointer to the object that is the target of the
     *            method call.
     * \param[in] method - Pointer to the instance method that will be called.
     */
	BoundMemberProcedure2(ClassType *targetObject, void (ClassType::*method)(Argument1Type, Argument2Type))
	: m_targetObject(targetObject), m_method(method)
	{}

    /**
     * \brief Copy constructor.
     */
	BoundMemberProcedure2(BoundMemberProcedure2 const& other) = default;

    /**
     * \brief Destructor.
     */
	virtual ~BoundMemberProcedure2() = default;

    /**
     * \brief Call the instance method on the target object.
     *
     * \param[in] argument1 First argument to pass to the method.
     * \param[in] argument2 Second argument to pass to the method.
     */
	void operator()(Argument1Type argument1, Argument2Type argument2)
	{ (m_targetObject->*m_method)(argument1, argument2); }

    /**
     * \brief Copy assignment operator.
     */
	BoundMemberProcedure2& operator=(BoundMemberProcedure2 const& other) = default;

private:
	ClassType *m_targetObject;
	void (ClassType::*m_method)(Argument1Type, Argument2Type);

	BoundMemberProcedure2();
};

/**
 * \brief Single argument bound member method factory returning non-void.
 * 
 * \deprecated This feature was created prior to the wide acceptance of the
 *             C++11 standard and its much more capable \c std::bind() facility.
 *             As such, these calls and their corresponding template classes
 *             will be removed in the next major release of Foundation.
 *
 * \param targetObject Object instance that contains the method to call.
 * \param method Pointer to the instance method to call.
 */
template<class ClassType, typename ReturnType, typename ArgumentType>
BoundMemberFunction1<ClassType, ReturnType, ArgumentType> BoundMember(ClassType *targetObject, ReturnType (ClassType::*method)(ArgumentType))
{
	return BoundMemberFunction1<ClassType, ReturnType, ArgumentType>(targetObject, method);
}

/**
 * \brief Single argument bound member method factory returning void (i.e., a \e procedure).
 *
 * \deprecated This feature was created prior to the wide acceptance of the
 *             C++11 standard and its much more capable \c std::bind() facility.
 *             As such, these calls and their corresponding template classes
 *             will be removed in the next major release of Foundation.
 *
 * \param targetObject Object instance that contains the method to call.
 * \param method Pointer to the bound member method to call.
 */
template<class ClassType, typename ArgumentType>
BoundMemberProcedure1<ClassType, ArgumentType> BoundMember(ClassType *targetObject, void (ClassType::*method)(ArgumentType))
{
	return BoundMemberProcedure1<ClassType, ArgumentType>(targetObject, method);
}

/**
 * \brief Dual argument bound member method factory returning non-void.
 * 
 * \deprecated This feature was created prior to the wide acceptance of the
 *             C++11 standard and its much more capable \c std::bind() facility.
 *             As such, these calls and their corresponding template classes
 *             will be removed in the next major release of Foundation.
 *
 * \param targetObject Object instance that contains the method to call.
 * \param method Pointer to the instance method to call.
 */
template<class ClassType, typename ReturnType, typename Argument1Type, typename Argument2Type>
BoundMemberFunction2<ClassType, ReturnType, Argument1Type, Argument2Type> BoundMember(ClassType *targetObject, ReturnType (ClassType::*method)(Argument1Type, Argument2Type))
{
	return BoundMemberFunction2<ClassType, ReturnType, Argument1Type, Argument2Type>(targetObject, method);
}

/**
 * \brief Dual argument bound member method factory returning void (i.e., a \e procedure).
 * 
 * \deprecated This feature was created prior to the wide acceptance of the
 *             C++11 standard and its much more capable \c std::bind() facility.
 *             As such, these calls and their corresponding template classes
 *             will be removed in the next major release of Foundation.
 * 
 * \param targetObject Object instance that contains the method to call.
 * \param method Pointer to the bound member method to call.
 */
template<class ClassType, typename Argument1Type, typename Argument2Type>
BoundMemberProcedure2<ClassType, Argument1Type, Argument2Type> BoundMember(ClassType *targetObject, void (ClassType::*method)(Argument1Type, Argument2Type))
{
	return BoundMemberProcedure2<ClassType, Argument1Type, Argument2Type>(targetObject, method);
}

} // end namespace CoreKit

#endif /* _FOUNDATION_COREKIT_BOUNDMEMBER_H_ */

// vim: set ts=4 sw=4 expandtab:
