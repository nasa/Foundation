/**
 * \file CanBusIo.h
 * \brief Definition of the CanBusIo Class
 * \date 2012-09-21 09:01:54
 * \author Rolando J. Nieves
 */
#if !defined(EA_E9021946_B4D0_46e3_8EC2_473076B0090E__INCLUDED_)
#define EA_E9021946_B4D0_46e3_8EC2_473076B0090E__INCLUDED_

#include <sys/socket.h>
#include <linux/can.h>
#include <string>

#include <CoreKit/InputSource.h>
#include <CoreKit/InterruptListener.h>
#include <CoreKit/RunLoop.h>
#include <CoreKit/factory.h>

#include "CanBusFrameCallback.h"

#define RF_CBK_MAX_FILTER_COUNT (32u)

namespace CanBusKit
{
	/**
	 * \brief CAN Bus activity event source.
	 * \author Rolando J. Nieves
	 * \date 2012-08-20 15:22:03
	 */
	class CanBusIo : public CoreKit::InputSource, public CoreKit::InterruptListener
	{
		RF_CK_FACTORY_COMPATIBLE(CanBusIo);
	public:
		CanBusIo(std::string const& canIfName, CoreKit::RunLoop* theRunLoop);
		CanBusIo(std::string const& canIfName, CoreKit::RunLoop* theRunLoop, std::vector<struct can_filter> const& inputFilter);
		virtual ~CanBusIo();

		inline std::string const& canBusIfName() const { return m_canBusIfName; }
		inline size_t canFilterCount() const { return m_canFilterCount; }
		void addCanFrameCallback(CanBusFrameCallback* theCallback);
		virtual int fileDescriptor() const;
		virtual InterruptListener* interruptListener() const;
		virtual void inputAvailableFrom(InputSource* theInputSource);
		virtual void fireCallback();
		void sendCanFrame(struct can_frame* theFrame);
		void startCan();
		void stopCan();

	private:
		enum { CREATED, STARTED, REGISTERED } m_canIfState;
		std::vector<CanBusKit::CanBusFrameCallback*> m_callbacks;
		int m_canBusIfIndex;
		int m_canBusFd;
		struct can_filter m_canFilters[RF_CBK_MAX_FILTER_COUNT];
		size_t m_canFilterCount;
		std::string m_canBusIfName;
		CoreKit::RunLoop *m_runLoop;
		CanBusFrameNotification m_prototypeNotif;

		void decipherCanBusIfIndex();

	};

}
#endif // !defined(EA_E9021946_B4D0_46e3_8EC2_473076B0090E__INCLUDED_)
