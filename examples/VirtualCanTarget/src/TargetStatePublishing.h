#ifndef _VIRTUAL_CAN_TARGET_SRC_TARGET_STATE_PUBLISHING_H_
#define _VIRTUAL_CAN_TARGET_SRC_TARGET_STATE_PUBLISHING_H_

#include <memory>

#include <CanBusKit/CanBusKit.h>

#include <TargetStateRepository.h>


class TargetStatePublishing
{
public:
    TargetStatePublishing(std::shared_ptr< TargetStateRepository > repository, std::shared_ptr< CanBusKit::CanBusIo > canIf);

    TargetStatePublishing(TargetStatePublishing const& other) = delete;
    
    TargetStatePublishing(TargetStatePublishing&& other) = delete;

    virtual ~TargetStatePublishing() = default;

    void sendUpdate();

private:
    std::shared_ptr< TargetStateRepository > m_repository;
    std::shared_ptr< CanBusKit::CanBusIo > m_canIf;
};

#endif /* !_VIRTUAL_CAN_TARGET_SRC_TARGET_STATE_PUBLISHING_H_ */

// vim: set ts=4 sw=4 expandtab:
