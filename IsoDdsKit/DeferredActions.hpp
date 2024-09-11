/**
 * \file IsoDdsKit/DeferredActions.hpp
 * \brief Contains the definition of the \c IsoDdsKit::DeferredActions class.
 * \author Rolando J. Nieves
 * \date 2024-08-23 14:57:25
 */

#ifndef _FOUNDATION_ISODDSKIT_DEFERREDACTIONS_HPP_
#define _FOUNDATION_ISODDSKIT_DEFERREDACTIONS_HPP_

#include <future>
#include <mutex>
#include <deque>


namespace IsoDdsKit
{

/**
 * \brief Container for deferred action calls.
 *
 * Instances of this class are able to accept function closures that can be
 * executed at a later time. The closures must be in the form of C++
 * \c std::future<> instances.
 */
class DeferredActions
{
public:
    /**
     * \brief Type alias for the action closures.
     */
    using ActionFuture = std::future< void >;

private:
    using ActionQueue = std::deque< ActionFuture >;
    using LockGuard = std::lock_guard< std::mutex >;

    std::mutex m_queueMutex;
    ActionQueue m_queue;

public:
    /**
     * \brief Default constructor.
     */
    DeferredActions() = default;

    DeferredActions(DeferredActions const& other) = delete;
    DeferredActions(DeferredActions&& other) = delete;

    /**
     * \brief Default destructor.
     */
    virtual ~DeferredActions() = default;

    /**
     * \brief Add a closure for execution at a later time.
     *
     * \param actionFuture Move reference to the deferred action closure.
     */
    void add(ActionFuture&& actionFuture);

    /**
     * \brief Execute the next deferred closure.
     *
     * This method calls the next deferred closure based on the order in which
     * they were added.
     *
     * \return \c true if more deferred actions remain; \c false otherwise.
     */
    bool dispatch();

    /**
     * \brief Execute all pending deferred closures.
     *
     * This method calls all remaining deferred closures based on the order in
     * which they were added.
     */
    void dispatchAll();

    DeferredActions& operator=(DeferredActions const& other) = delete;
    DeferredActions& operator=(DeferredActions&& other) = delete;
};

} // end namespace IsoDdsKit

#endif /* !_FOUNDATION_ISODDSKIT_DEFERREDACTIONS_HPP_ */

// vim: set ts=4 sw=4 expandtab:
