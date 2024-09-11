/**
 * \file IsoDdsKit/EmitStatusAction.hpp
 * \brief Contains the definition of the \c IsoDdsKit::EmitStatusAction class.
 * \author Rolando J. Nieves
 * \date 2024-08-23 13:53:27
 */

#ifndef _FOUNDATION_ISODDSKIT_EMITSTATUSACTION_HPP_
#define _FOUNDATION_ISODDSKIT_EMITSTATUSACTION_HPP_

#include <dds/core/status/Status.hpp>

#include <IsoDdsKit/ValueDistribution.hpp>


namespace IsoDdsKit
{

/**
 * \brief Closure that retains a status value for broadcast.
 *
 * Instances of this class are used to store all information necessary to emit
 * copies of entity status values via an event distributor. The context for the
 * status broadcast includes a copy of the status value and the event
 * distributor used to broadcast to interested listeners. The class is
 * primarily used to create closures that can be deferred for later execution,
 * usually to coordinate between a DDS provider's listener thread and the
 * application main thread.
 *
 * The class definition also includes an inner class that can be used to
 * fabricate instances with part of the closure context already established.
 */
template< typename StatusType >
class EmitStatusAction
{
public:
    /**
     * \brief Type alias for a shared pointer to the event distributor.
     */
    using StatusDistributionPtr = std::shared_ptr< ValueDistribution< StatusType > >;

private:
    StatusDistributionPtr m_statusDistribution;
    StatusType m_status;

public:
    /**
     * \brief Factory for creating status broadcast closures.
     *
     * This factory class serves as a partial binding for the status broadcast
     * closure, by remembering the event distributor to associate with the
     * created closures.
     */
    class FactoryCall
    {
    private:
        StatusDistributionPtr m_statusDistribution;
    
    public:
        /**
         * \brief Remember the event distributor to use when creating closures.
         *
         * \param statusDistribution Event distributor to bind to created
         *        closures.
         */
        explicit FactoryCall(StatusDistributionPtr statusDistribution);

        /**
         * \brief Copy constructor.
         */
        FactoryCall(FactoryCall const& other);

        /**
         * \brief Move constructor.
         */
        FactoryCall(FactoryCall&& other);

        /**
         * \brief Default destructor.
         */
        virtual ~FactoryCall() = default;

        /**
         * \brief Create new status broadcast closure.
         *
         * This method combines the event distributor provided at construction
         * time with the status value provided in the argument list to
         * create a new status broadcast closure.
         *
         * \param status Status value that will be broadcast by the created
         *        closure.
         *
         * \return New status broadcast closure.
         */
        EmitStatusAction createNew(StatusType const& status);
    };

    /**
     * \brief Create the status broadcast closure.
     *
     * References to both the status value and the event distributor are held
     * through the lifetime of this closure object.
     *
     * \param status Status value to broadcast
     * \param statusDistribution Event distributor to use.
     */
    explicit EmitStatusAction(StatusType const& status, StatusDistributionPtr statusDistribution);

    /**
     * \brief Move constructor.
     *
     * This constructor is required to make these closures compatible with
     * \c std::future<>
     */
    EmitStatusAction(EmitStatusAction&& other);

    EmitStatusAction(EmitStatusAction const& other) = delete;

    /**
     * \brief Default destructor.
     */
    virtual ~EmitStatusAction() = default;

    /**
     * \brief Broadcast status value to interested listeners.
     */
    void operator()();
};

template< typename StatusType >
EmitStatusAction< StatusType >::FactoryCall::FactoryCall(StatusDistributionPtr statusDistribution):
    m_statusDistribution(statusDistribution)
{
    if (!m_statusDistribution)
    {
        throw std::logic_error("EmitStatusAction::FactoryCall: StatusDistribution is null.");
    }
}

template< typename StatusType >
EmitStatusAction< StatusType >::FactoryCall::FactoryCall(FactoryCall const& other):
    m_statusDistribution(other.m_statusDistribution)
{}

template< typename StatusType >
EmitStatusAction< StatusType >::FactoryCall::FactoryCall(FactoryCall&& other):
    m_statusDistribution(std::move(other.m_statusDistribution))
{}

template< typename StatusType >
EmitStatusAction< StatusType >
EmitStatusAction< StatusType >::FactoryCall::createNew(StatusType const& status)
{
    return EmitStatusAction< StatusType >(status, m_statusDistribution);
}

template< typename StatusType >
EmitStatusAction< StatusType >::EmitStatusAction(StatusType const& status, StatusDistributionPtr statusDistribution):
    m_status(status),
    m_statusDistribution(statusDistribution)
{}

template< typename StatusType >
EmitStatusAction< StatusType >::EmitStatusAction(EmitStatusAction< StatusType >&& other):
    m_status(std::move(other.m_status)),
    m_statusDistribution(std::move(other.m_statusDistribution))
{}

template< typename StatusType >
void
EmitStatusAction< StatusType >::operator()()
{
    if (!m_statusDistribution)
    {
        throw std::runtime_error("EmitStatusAction: SampleDistribution is null.");
    }

    m_statusDistribution->emit(m_status);
}

} // end namespace IsoDdsKit

#endif /* !_FOUNDATION_ISODDSKIT_EMITSTATUSACTION_HPP_ */

// vim: set ts=4 sw=4 expandtab:
