#include "CommandDispatch.h"


using CoreKit::EventInputSource;
using CoreKit::InputSource;

void
CommandDispatch::initialize(std::shared_ptr< EventInputSource > cmdEvent)
{
    m_cmdEvent = cmdEvent;
}


void
CommandDispatch::inputAvailableFrom(CoreKit::InputSource *source)
{
    while (!m_cmdQueue.empty())
    {
        for (auto& aHandler : m_handlers)
        {
            aHandler(m_cmdQueue.front());
            m_cmdQueue.pop();
        }
    }
}


void
CommandDispatch::post(std::string const& target, std::string const& action)
{
    m_cmdQueue.push(target + " " + action);
    if (m_cmdEvent)
    {
        m_cmdEvent->assertEvent();
    }
}


void
CommandDispatch::terminate()
{
    m_cmdEvent.reset();
}

// vim: set ts=4 sw=4 expandtab:
