#include "TargetStateChangeEventDispatch.h"


void
TargetStateChangeEventDispatch::addChange(canid_t canId, std::uint8_t value)
{
    m_accumulatedChanges[canId] = value;
}


void
TargetStateChangeEventDispatch::inputAvailableFrom(CoreKit::InputSource *source)
{
    for (auto& aHandler : m_eventHandlers)
    {
        aHandler(m_accumulatedChanges);
    }
    m_accumulatedChanges.clear();
}

// vim: set ts=4 sw=4 expandtab:
