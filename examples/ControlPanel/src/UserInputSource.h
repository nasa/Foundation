#ifndef _VIRTUAL_CAN_TARGET_SRC_USER_INPUT_SOURCE_H_
#define _VIRTUAL_CAN_TARGET_SRC_USER_INPUT_SOURCE_H_

#include <cstdio>
#include <memory>
#include <CoreKit/CoreKit.h>

#include <UserInputEventDispatch.h>


class UserInputSource : public CoreKit::InputSource
{
public:
    UserInputSource(std::shared_ptr< UserInputEventDispatch > dispatch);

    virtual ~UserInputSource() = default;
    
    std::shared_ptr< UserInputEventDispatch > dispatch() const { return m_dispatch; }
    
    virtual int fileDescriptor() const override
    { return fileno(stdin); }

    virtual void fireCallback() override
    { m_dispatch->inputAvailableFrom(this); }

    virtual CoreKit::InterruptListener* interruptListener() const override
    { return m_dispatch.get(); }

    int readNext();

private:
    std::shared_ptr< UserInputEventDispatch > m_dispatch;
};

#endif /* !_VIRTUAL_CAN_TARGET_SRC_USER_INPUT_SOURCE_H_ */

// vim: set ts=4 sw=4 expandtab:
