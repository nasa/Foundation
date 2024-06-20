/**
 * \file ThreadDelegate.h
 * \brief Definition of the ThreadDelegate Class
 * \date 2012-09-24 08:39:58
 * \author Rolando J. Nieves
 */
#if !defined(EA_D7428BD7_8AAB_469e_8FC6_899893F20C08__INCLUDED_)
#define EA_D7428BD7_8AAB_469e_8FC6_899893F20C08__INCLUDED_

namespace CoreKit
{
	class Thread;

	/**
	 * \brief Delegate Interface for Concurrent Task Behavior
	 *
	 * The \c ThreadDelegate interface gives foundation developers the ability
	 * to define concurrent task behavior independent of the entity used to
	 * host the concurrent task activities. Classes that implement this
	 * interface should treat the ThreadDelegate::doThreadLogic(Thread*) method
	 * and their custom logic entry point.
	 *
	 * \author Rolando J. Nieves
	 * \date 2012-09-07 14:05:05
	 */
	class ThreadDelegate
	{

	public:
		/**
		 * \brief Default Constructor for the \c ThreadDelegate Interface
		 */
		ThreadDelegate();
		/**
		 * \brief End of Destructor Chain for \c ThreadDelegate Instances.
		 */
		virtual ~ThreadDelegate();
		/**
		 * \brief Entry Point for Custom Task Logic
		 *
		 * The \c doThreadLogic() method is called on the \c ThreadDelegate
		 * derived instance with the intent to start the concurrent tasks's
		 * custom logic. A pointer to the \c Thread instance hosting this
		 * \c ThreadDelegate is passed as input, giving the custom task logic
		 * access to, among other things, the \c RunLoop instance created
		 * specifically for this concurrent task (see Thread::getRunLoop() ).
		 *
		 * \param theThread \c Thread instance hosting this delegate instance.
		 */
		virtual void doThreadLogic(Thread* theThread) = 0;

	};

}
#endif // !defined(EA_D7428BD7_8AAB_469e_8FC6_899893F20C08__INCLUDED_)
