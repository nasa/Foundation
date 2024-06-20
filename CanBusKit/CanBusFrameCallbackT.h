/**
 * \file CanBusFrameCallbackT.h
 * \brief Definition of the CanBusFrameCallbackT Class
 * \date 2012-09-21 09:01:53
 * \author Rolando J. Nieves
 */
#if !defined(EA_EA8CEBBB_5A01_4d26_AD69_64FE52E2093C__INCLUDED_)
#define EA_EA8CEBBB_5A01_4d26_AD69_64FE52E2093C__INCLUDED_

#include "CanBusFrameCallback.h"

namespace CanBusKit
{
	/**
	 * \brief Template for callback objects compatible with \c CanBusKit
	 * \author Rolando J. Nieves
	 * \date 2012-09-17 16:05:50
	 */
	template<class TargetType>
	class CanBusFrameCallbackT : public CanBusKit::CanBusFrameCallback
	{

	public:
		CanBusFrameCallbackT(TargetType callbackTarget)
		: m_callbackTarget(callbackTarget)
		{

		}

		CanBusFrameCallbackT(CanBusFrameCallbackT const& other)
		: m_callbackTarget(other.m_callbackTarget)
		{

		}

		virtual ~CanBusFrameCallbackT()
		{

		}

		virtual void operator()(CanBusFrameNotification* theNotification)
		{
			m_callbackTarget(theNotification);
		}

	private:

		TargetType m_callbackTarget;
	};

	template<class TargetType>
	CanBusFrameCallback* newCanBusFrameCallback(TargetType callbackTarget)
	{
		return new CanBusFrameCallbackT<TargetType>(callbackTarget);
	}
}
#endif // !defined(EA_EA8CEBBB_5A01_4d26_AD69_64FE52E2093C__INCLUDED_)
