#include "TargetStateRepository.h"


TargetStateRepository::TargetStateRepository(
    EventDispatchPtr eventDispatch,
    EventSourcePtr eventSource
):
    m_eventDispatch(eventDispatch),
    m_eventSource(eventSource)
{}


std::uint8_t
TargetStateRepository::get(canid_t canId)
{
    std::uint8_t result = 0u;

    auto cvtIter = m_currentState.find(canId);
    if (cvtIter != m_currentState.end())
    {
        result = (*cvtIter).second;
    }

    return result;
}


void
TargetStateRepository::set(canid_t canId, std::uint8_t value)
{
    m_currentState[canId] = value;
    m_eventDispatch->addChange(canId, value);
    m_eventSource->assertEvent();
}

// vim: set ts=4 sw=4 expandtab:
