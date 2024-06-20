#ifndef _VIRTUAL_CAN_TARGET_SRC_TARGET_STATE_CHANGE_EVENT_DISPATCH_H_
#define _VIRTUAL_CAN_TARGET_SRC_TARGET_STATE_CHANGE_EVENT_DISPATCH_H_

#include <functional>
#include <vector>
#include <CoreKit/CoreKit.h>

#include <TargetStateInformation.h>


class TargetStateChangeEventDispatch : public CoreKit::InterruptListener
{
public:
    TargetStateChangeEventDispatch() = default;

    virtual ~TargetStateChangeEventDispatch() = default;

    TargetStateChangeEventDispatch(TargetStateChangeEventDispatch const& other) = delete;

    TargetStateChangeEventDispatch(TargetStateChangeEventDispatch&& other) = delete;

    void addChange(canid_t canId, std::uint8_t value);

    template< typename EventHandler >
    void addChangeEventHandler(EventHandler&& handler)
    {
        m_eventHandlers.emplace_back(std::forward< EventHandler >(handler));
    }

    virtual void inputAvailableFrom(CoreKit::InputSource *source) override;

    TargetStateChangeEventDispatch& operator=(TargetStateChangeEventDispatch const& other) = delete;

    TargetStateChangeEventDispatch& operator=(TargetStateChangeEventDispatch&& other) = delete;

private:
    using HandlerList = std::vector< std::function< void (TargetStateInformation const&) > >;

    TargetStateInformation m_accumulatedChanges;
    HandlerList m_eventHandlers;
};

#endif /* !_VIRTUAL_CAN_TARGET_SRC_TARGET_STATE_CHANGE_EVENT_DISPATCH_H_ */

// vim: set ts=4 sw=4 expandtab:
