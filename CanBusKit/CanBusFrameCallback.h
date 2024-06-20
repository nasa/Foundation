/**
 * \file CanBusFrameCallback.h
 * \brief Definition of the CanBusFrameCallback Class
 * \date 2012-09-21 09:01:53
 * \author Rolando J. Nieves
 */
#if !defined(EA_AB5C364B_3CBF_48ab_8C70_1A67E4E86123__INCLUDED_)
#define EA_AB5C364B_3CBF_48ab_8C70_1A67E4E86123__INCLUDED_

#include "CanBusFrameNotification.h"

namespace CanBusKit
{
	/**
	 * 
	 * \author rnieves
	 * \date 17-Sep-2012 4:05:28 PM
	 */
	class CanBusFrameCallback
	{

	public:
		CanBusFrameCallback();
		virtual ~CanBusFrameCallback();

		virtual void operator()(CanBusFrameNotification* theNotification) =0;

	};

}
#endif // !defined(EA_AB5C364B_3CBF_48ab_8C70_1A67E4E86123__INCLUDED_)
