#ifndef _VIRTUAL_CAN_TARGET_SRC_FOCUS_RECEIVER_HH_
#define _VIRTUAL_CAN_TARGET_SRC_FOCUS_RECEIVER_HH_

class FocusReceiver
{
public:
    virtual void gainFocus() = 0;

    virtual void loseFocus() = 0;
};

#endif /* !_VIRTUAL_CAN_TARGET_SRC_FOCUS_RECEIVER_HH_ */

// vim: set ts=4 sw=4 expandtab:
