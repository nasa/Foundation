#ifndef _VIRTUAL_CAN_TARGET_SRC_INPUT_RECEIVER_HH_
#define _VIRTUAL_CAN_TARGET_SRC_INPUT_RECEIVER_HH_

class InputReceiver
{
public:
    virtual void handleInput(int input) = 0;
};

#endif /* !_VIRTUAL_CAN_TARGET_SRC_INPUT_RECEIVER_HH_ */

// vim: set ts=4 sw=4 expandtab:
