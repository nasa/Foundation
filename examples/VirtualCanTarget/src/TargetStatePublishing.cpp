#include <Identifiers.h>

#include "TargetStatePublishing.h"


namespace
{
    const std::vector< canid_t > PUBLISH_LIST({ VctIdentifiers::ALPHA, VctIdentifiers::BRAVO, VctIdentifiers::CHARLIE });
}

TargetStatePublishing::TargetStatePublishing(
    std::shared_ptr< TargetStateRepository > repository,
    std::shared_ptr< CanBusKit::CanBusIo > canIf
):
    m_canIf(canIf),
    m_repository(repository)
{}


void
TargetStatePublishing::sendUpdate()
{
    struct can_frame aFrame;

    for (auto aPublishId : PUBLISH_LIST)
    {
        memset(&aFrame, 0x00, sizeof(can_frame));
        aFrame.can_id = aPublishId;
        aFrame.can_dlc = 1;
        aFrame.data[0] = m_repository->get(aPublishId);

        m_canIf->sendCanFrame(&aFrame);
    }
}

// vim: set ts=4 sw=4 expandtab:
