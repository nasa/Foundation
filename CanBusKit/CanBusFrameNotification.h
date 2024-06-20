/**
 * \file CanBusFrameNotification.h
 * \brief Definition of the CanBusFrameNotification Class
 * \date 2012-09-21 09:01:54
 * \author Rolando J. Nieves
 */
#if !defined(EA_1C075C88_1A35_4309_9140_3C4A3C80DDE7__INCLUDED_)
#define EA_1C075C88_1A35_4309_9140_3C4A3C80DDE7__INCLUDED_

#include <time.h>
#include <stdint.h>
#include <vector>

#include <CoreKit/CoreKit.h>

namespace CanBusKit
{
	typedef CoreKit::StaticByteVector<8> CanPayloadVector;

	/**
	 * \brief Container for all information pertaining to a CAN Bus event.
	 * \author Rolando J. Nieves
	 * \date 2012-09-17 16:02:32
	 */
	struct CanBusFrameNotification
	{

	public:
		timespec const& acqTime;
		uint32_t const& canId;
		CanPayloadVector const& canPayload;

		CanBusFrameNotification(uint32_t theCanId, timespec const& theAcqTime, CanPayloadVector const& thePayload);

		inline bool effMessage() const { return m_effMessage; }
		inline bool rtrMessage() const { return m_rtrMessage; }
		inline bool errFrame() const { return m_errFrame; }
		
	private:
		timespec m_acqTime;
		uint32_t m_canId;
		CanPayloadVector m_canPayload;
		bool m_effMessage;
		bool m_rtrMessage;
		bool m_errFrame;

		CanBusFrameNotification();

		void decodeCanId();
		
		friend class CanBusIo;
	};

}
#endif // !defined(EA_1C075C88_1A35_4309_9140_3C4A3C80DDE7__INCLUDED_)
