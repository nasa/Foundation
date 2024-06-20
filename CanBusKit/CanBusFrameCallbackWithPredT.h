/**
 * \file CanBusFrameCallbackWithPredT.h
 * \brief Definition of the CanBusFrameCallbackWithPredT Class
 * \date 2012-09-21 09:01:53
 * \author Rolando J. Nieves
 */
#if !defined(EA_871D88C6_A4E1_45b2_943A_844F12E84819__INCLUDED_)
#define EA_871D88C6_A4E1_45b2_943A_844F12E84819__INCLUDED_

#include <linux/can.h>

#include "CanBusFrameCallbackT.h"

namespace CanBusKit
{
	/**
	 * \brief Template for predicated callbacks compatible with \c CanBusKit.
	 * \author Rolando J. Nieves
	 * \date 2012-09-17 16:06:22
	 */
	template<class TargetType, class PredicateType>
	class CanBusFrameCallbackWithPredT : public CanBusFrameCallbackT<TargetType>
	{

	public:
		CanBusFrameCallbackWithPredT(TargetType callbackTarget, PredicateType conditionPredicate)
		: CanBusFrameCallbackT<TargetType>(callbackTarget), m_conditionPredicate(conditionPredicate)
		{

		}

		virtual ~CanBusFrameCallbackWithPredT()
		{

		}

		virtual void operator()(CanBusFrameNotification* theNotification)
		{
			if (m_conditionPredicate(theNotification) == true)
			{
				CanBusFrameCallbackT<TargetType>::operator()(theNotification);
			}
		}

	private:

		PredicateType m_conditionPredicate;
	};


	struct CanIdMatch
	{
	public:

		const canid_t canId;

		inline CanIdMatch(canid_t theCanId) : canId(theCanId) {};
		inline CanIdMatch(CanIdMatch const& other) : canId(other.canId) {};
		inline bool operator()(CanBusFrameNotification *theNotif) { return theNotif->canId == canId; };
	};


	struct CanIdRange
	{
	public:

		const canid_t startCanId;
		const canid_t endCanId;

		inline CanIdRange(canid_t theStartCanId, canid_t theEndCanId) : startCanId(theStartCanId), endCanId(theEndCanId) {};
		inline CanIdRange(CanIdRange const& other) : startCanId(other.startCanId), endCanId(other.endCanId) {};
		inline bool operator()(CanBusFrameNotification *theNotif)
		{ return (theNotif->canId >= startCanId) && (theNotif->canId <= endCanId); }
	};


	template<class TargetType, class PredicateType>
	CanBusFrameCallback* newCanBusFrameCallbackWithPred(TargetType callbackTarget, PredicateType conditionPredicate)
	{
		return new CanBusFrameCallbackWithPredT<TargetType,PredicateType>(callbackTarget, conditionPredicate);
	}
}
#endif // !defined(EA_871D88C6_A4E1_45b2_943A_844F12E84819__INCLUDED_)
