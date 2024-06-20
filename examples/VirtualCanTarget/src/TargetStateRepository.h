#ifndef _VIRTUAL_CAN_TARGET_SRC_TARGET_STATE_REPOSITORY_H_
#define _VIRTUAL_CAN_TARGET_SRC_TARGET_STATE_REPOSITORY_H_

#include <memory>

#include <CanBusKit/CanBusKit.h>

#include <TargetStateChangeEventDispatch.h>
#include <TargetStateInformation.h>


class TargetStateRepository
{
public:
    using EventDispatchPtr = std::shared_ptr< TargetStateChangeEventDispatch >;
    using EventSourcePtr = std::shared_ptr< CoreKit::EventInputSource >;

    TargetStateRepository(EventDispatchPtr eventDispatch, EventSourcePtr eventSource);

    TargetStateRepository(TargetStateRepository const& other) = delete;

    TargetStateRepository(TargetStateRepository&& other) = delete;

    virtual ~TargetStateRepository() = default;

    inline EventDispatchPtr eventDispatch() const { return m_eventDispatch; }

    std::uint8_t get(canid_t canId);

    void set(canid_t canId, std::uint8_t value);

    TargetStateRepository& operator=(TargetStateRepository const& other) = delete;
    
    TargetStateRepository& operator=(TargetStateRepository&& other) = delete;

private:
    TargetStateInformation m_currentState;
    EventDispatchPtr m_eventDispatch;
    EventSourcePtr m_eventSource;
};

#endif /* !_VIRTUAL_CAN_TARGET_SRC_TARGET_STATE_REPOSITORY_H_ */

// vim: set ts=4 sw=4 expandtab:
