/**
 * \file BlockGuard.h
 * \brief Contains the definition for the \c CoreKit::BlockGuard class.
 * \date 2013-09-16 10:32:00
 * \author Rolando J. Nieves
 */

#ifndef _FOUNDATION_COREKIT_BLOCKGUARD_H_
#define _FOUNDATION_COREKIT_BLOCKGUARD_H_

#include <pthread.h>

namespace CoreKit
{

/**
 * \brief Simple mutex guard for a code block.
 *
 * Instances of this class, upon construction, acquire an exclusive lock on the
 * mutex specified and hold it until the instance is destroyed. The typical use
 * case for this class is to serialize entry into a particular code block
 * among threads.
 */
class BlockGuard
{
public:
    /**
     * \brief Block until mutex is acquired.
     *
     * The constructor will block until it is able to acquire the mutex.
     *
     * \param[in] guardMutex Pthread mutex to acquire immediately.
     */
    explicit BlockGuard(pthread_mutex_t *guardMutex);

    /**
     * \brief Release the mutex specified at construction time.
     */
    virtual ~BlockGuard();

private:
    pthread_mutex_t *m_guardMutex;
};

} /* namespace CoreKit */

#endif /* _FOUNDATION_COREKIT_BLOCKGUARD_H_ */

// vim: set ts=4 sw=4 expandtab:
