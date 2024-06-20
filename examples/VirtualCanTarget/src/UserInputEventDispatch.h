#ifndef _VIRTUAL_CAN_TARGET_USER_INPUT_EVENT_DISPATCH_H_
#define _VIRTUAL_CAN_TARGET_USER_INPUT_EVENT_DISPATCH_H_

#include <functional>
#include <queue>
#include <vector>

#include <CoreKit/CoreKit.h>


class UserInputEventDispatch : public CoreKit::InterruptListener
{
public:
    UserInputEventDispatch() = default;

    virtual ~UserInputEventDispatch() = default;

    UserInputEventDispatch(UserInputEventDispatch const& other) = delete;

    UserInputEventDispatch(UserInputEventDispatch&& other) = delete;

    template< typename EventHandler >
    void addUserInputHandler(EventHandler&& handler)
    { m_eventHandlers.emplace_back(std::forward< EventHandler >(handler)); }

    virtual void inputAvailableFrom(CoreKit::InputSource *source) override;
    
    UserInputEventDispatch& operator=(UserInputEventDispatch const& other) = delete;

    UserInputEventDispatch& operator=(UserInputEventDispatch&& other) = delete;

private:
    using HandlerList = std::vector< std::function< void (int) > >;

    HandlerList m_eventHandlers;
};

#endif /* !_VIRTUAL_CAN_TARGET_USER_INPUT_EVENT_DISPATCH_H_ */

// vim: set ts=4 sw=4 expandtab:
