/**
 * \file BlockGuard.cpp
 * \brief Contains the implementation for the \c CoreKit::BlockGuard class.
 * \date 2013-09-16 10:34:00
 * \author Rolando J. Nieves
 */

#include "BlockGuard.h"

namespace CoreKit {


BlockGuard::BlockGuard(pthread_mutex_t *guardMutex):
    m_guardMutex(guardMutex)
{
    if (m_guardMutex != nullptr)
    {
        pthread_mutex_lock(m_guardMutex);
    }
}



BlockGuard::~BlockGuard()
{
	pthread_mutex_unlock(m_guardMutex);
	m_guardMutex = nullptr;
}

} /* namespace CoreKit */

// vim: set ts=4 sw=4 expandtab:
