#ifndef _VIRTUAL_CAN_TARGET_SRC_IDENTIFIERS_H_
#define _VIRTUAL_CAN_TARGET_SRC_IDENTIFIERS_H_

#include <CanBusKit/CanBusKit.h>


namespace VctIdentifiers
{
    constexpr canid_t ALPHA = 0x100;
    constexpr canid_t BRAVO = 0x101;
    constexpr canid_t CHARLIE = 0x102;
    constexpr canid_t SW0 = 0x200;
    constexpr canid_t SW1 = 0x201;
    constexpr canid_t SW2 = 0x202;
}

#endif /* !_VIRTUAL_CAN_TARGET_SRC_IDENTIFIERS_H_ */

// vim: set ts=4 sw=4 expandtab:
