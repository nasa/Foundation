/**
 * \file SynchronizedRunLoop.h
 * \brief Definition of the SynchronizedRunLoop Class
 * \date 2012-11-16 10:51:20
 * \author Rolando J. Nieves
 */
#if !defined(EA_3BE893D6_8565_4698_AF54_1C11065702D2__INCLUDED_)
#define EA_3BE893D6_8565_4698_AF54_1C11065702D2__INCLUDED_

#include <semaphore.h>

#include <CoreKit/RunLoop.h>

namespace CoreKit
{
    /**
     * \brief Run loop slaved to an external synchronization object.
     * \author Rolando J. Nieves
     * \date 2012-11-16 10:13:00
     */
    class SynchronizedRunLoop : public CoreKit::RunLoop
    {

    public:
        /**
         * \brief Initialize the synchronization object that this run loop uses.
         *
         * \param[in] runLoopSyncObj - Synchronization object to use.
         */
        explicit SynchronizedRunLoop(sem_t* runLoopSyncObj);

        /**
         * \brief Destructor.
         */
        virtual ~SynchronizedRunLoop() = default;

        /**
         * \brief Start the run loop.
         */
        virtual void run();

    private:
        sem_t *m_runLoopSyncObj;
    };

}
#endif // !defined(EA_3BE893D6_8565_4698_AF54_1C11065702D2__INCLUDED_)

// vim: set ts=4 sw=4 expandtab:
