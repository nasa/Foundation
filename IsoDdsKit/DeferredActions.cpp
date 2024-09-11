/**
 * \file IsoDdsKit/DeferredActions.cpp
 * \brief Contains the implementation for the \c IsoDdsKit::DeferredActions class.
 * \author Rolando J. Nieves
 * \date 2024-08-23 15:13:08
 */

#include "DeferredActions.hpp"


namespace IsoDdsKit
{


void
DeferredActions::add(ActionFuture&& actionFuture)
{
    LockGuard guard(m_queueMutex);

    m_queue.emplace_back(std::forward< ActionFuture >(actionFuture));
}


bool
DeferredActions::dispatch()
{
    LockGuard guard(m_queueMutex);

    bool result = !m_queue.empty();
    if (result)
    {
        try
        {
            m_queue.front().get();
        }
        catch (std::exception& ex)
        {}

        m_queue.pop_front();
        result = !m_queue.empty();
    }

    return result;
}


void
DeferredActions::dispatchAll()
{
    while (dispatch()) {}
}

} // end namespace IsoDdsKit

// vim: set ts=4 sw=4 expandtab:
