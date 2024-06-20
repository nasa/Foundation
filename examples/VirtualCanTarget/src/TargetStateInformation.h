#ifndef _VIRTUAL_CAN_TARGET_SRC_TARGET_STATE_INFORMATION_H_
#define _VIRTUAL_CAN_TARGET_SRC_TARGET_STATE_INFORMATION_H_

#include <cstdint>
#include <map>

#include <CanBusKit/CanBusKit.h>


using TargetStateInformation = std::map< canid_t, std::uint8_t >;

#endif /* !_VIRTUAL_CAN_TARGET_SRC_TARGET_STATE_INFORMATION_H_ */

// vim: set ts=4 sw=4 expandtab:
