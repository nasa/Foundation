/**
 * \file IsoDdsKit/ReaderContext.hpp
 * \brief Contains the definition of the \c IsoDdsKit::ReaderContext class.
 * \author Rolando J. Nieves
 * \date 2024-08-22 12:30:17
 */

#ifndef _FOUNDATION_ISODDSKIT_READERCONTEXT_HPP_
#define _FOUNDATION_ISODDSKIT_READERCONTEXT_HPP_

#include <memory>

#include <dds/core/status/Status.hpp>
#include <dds/domain/DomainParticipant.hpp>
#include <dds/topic/Topic.hpp>
#include <dds/sub/DataReader.hpp>
#include <dds/sub/Subscriber.hpp>

#include <CoreKit/CoreKit.h>

#include <IsoDdsKit/ValueDistribution.hpp>
#include <IsoDdsKit/ReaderStimuliMux.hpp>
#include <IsoDdsKit/status_types.hpp>


namespace IsoDdsKit
{

/**
 * \brief Group of objects that integrate a data reader with the Foundation run loop.
 *
 * Instances of this class are mostly a container to hold the group of objects
 * that let a \c dds::sub::DataReader instance integrate well with a Foundation
 * application's run loop. The instances, in addition to the reader itself,
 * include:
 * * The event distributor used to broadcast new reader samples
 * * The event distributors used to broadcast reader status changes
 * * The reader stimuli multiplexer used to schedule responses to reader
 *   stimuli with the run loop.
 */
template< typename SampleType >
class ReaderContext : public CoreKit::InterruptListener
{
public:
    /**
     * \brief Type alias to the reader data event distributor.
     */
    using SampleDistribution = ValueDistribution< SampleType >;

private:
    using Reader = dds::sub::DataReader< SampleType >;
    using SampleDistributionPtr = std::shared_ptr< SampleDistribution >;
    using SampleLostStatusDistributionPtr = std::shared_ptr< SampleLostStatusDistribution >;
    using SubscriptionMatchedStatusDistributionPtr = std::shared_ptr< SubscriptionMatchedStatusDistribution >;
    using ReaderMux = ReaderStimuliMux< SampleType >;

    Reader m_reader;
    ReaderMux m_readerMux;
    SampleDistributionPtr m_sampleDistribution;
    SampleLostStatusDistributionPtr m_sampleLostStatusDistribution;
    SubscriptionMatchedStatusDistributionPtr m_subscriptionMatchedStatusDistribution;

public:
    /**
     * \brief Create the data reader using the provided infrastructure.
     *
     * This constructor overload uses the default quality of service settings
     * configured in the subscriber at the time of the call.

     * \param subscriber \c dds::sub::Subscriber instance associated with the
     *        reader.
     * \param topic \c dds::topic::Topic instance associated with the reader.
     */
    explicit ReaderContext(
        dds::sub::Subscriber& subscriber,
        dds::topic::Topic< SampleType >& topic
    );

    /**
     * \brief Create the data reader using the provided infrastructure.
     *
     * This constructor overload uses the quality of service settings provided
     * in the argument list.

     * \param subscriber \c dds::sub::Subscriber instance associated with the
     *        reader.
     * \param topic \c dds::topic::Topic instance associated with the reader.
     * \param qos Quality of service settings to use when creating the reader.
     */
    explicit ReaderContext(
        dds::sub::Subscriber& subscriber,
        dds::topic::Topic< SampleType >& topic,
        dds::sub::qos::DataReaderQos const& qos
    );

    /**
     * \brief Close out the reader reference in this instance.
     */
    virtual ~ReaderContext();

    ReaderContext(ReaderContext const& other) = delete;
    ReaderContext(ReaderContext&& other) = delete;

    /**
     * \brief Register the reader with the application run loop.
     *
     * This method also registers the reader stimuli multiplexer as the sole
     * listener with the \c dds::sub::DataReader instance.
     *
     * \param runLoop Run loop to register the reader input source with.
     */
    void activate(CoreKit::RunLoop *runLoop);

    /**
     * \brief De-register the reader with the application loop.
     *
     * This method also removes the listener from the \c dds::sub::DataReader
     * instance managed by this object.
     *
     * \param runLoop Run loop where the reader input source is registered.
     */
    void deactivate(CoreKit::RunLoop *runLoop);

    /**
     * \brief Callback used by the reader multiplexer when stimuli occur.
     *
     * When the reader stimuli multiplexer detects activity via its implemented
     * reader listener interface, it generates an event with the application
     * run loop that eventually ends up as a call to this method.
     *
     * \param source Input source that generated the event.
     */
    virtual void inputAvailableFrom(CoreKit::InputSource *source) override;

    /**
     * \brief Access the data reader instance managed by this object.
     *
     * \return \c dds::sub::DataReader instance managed by this object.
     */
    inline Reader& reader()
    { return m_reader; }

    /**
     * \brief Access the reader sample event distributor instance.
     *
     * This method is typically used to register listeners interested in
     * receiving samples acquired from data readers.
     *
     * \return Reference to the reader sample event distributor.
     */
    SampleDistribution& sampleDistribution();

    /**
     * \brief Access the sample lost status event distributor instance.
     *
     * This method is typically used to register listeners interested in
     * receiving sample lost status updates.
     *
     * \return Reference to the reader sample lost status event distributor.
     */
    SampleLostStatusDistribution& sampleLostStatusDistribution();

    /**
     * \brief Access the subscription matched status event distributor instance.
     *
     * This method is typically used to register listeners interested in
     * receiving subscription matched status updates.
     *
     * \return Reference to the reader subscription matched status event
     *         distributor.
     */
    SubscriptionMatchedStatusDistribution& subscriptionMatchedStatusDistribution();
};


template< typename SampleType >
ReaderContext< SampleType >::ReaderContext(
    dds::sub::Subscriber& subscriber,
    dds::topic::Topic< SampleType >& topic
):
    m_reader(dds::core::null),
    m_readerMux(this)
{
    m_reader = Reader(subscriber, topic);
}


template< typename SampleType >
ReaderContext< SampleType >::ReaderContext(
    dds::sub::Subscriber& subscriber,
    dds::topic::Topic< SampleType >& topic,
    dds::sub::qos::DataReaderQos const& qos
):
    m_reader(dds::core::null),
    m_readerMux(this)
{
    m_reader = Reader(subscriber, topic, qos);
}


template< typename SampleType >
ReaderContext< SampleType >::~ReaderContext()
{
    m_reader.close();
}


template< typename SampleType >
void
ReaderContext< SampleType >::activate(CoreKit::RunLoop *runLoop)
{
    runLoop->registerInputSource(&m_readerMux);
    m_reader.listener(&m_readerMux, m_readerMux.statusMask());
}


template< typename SampleType >
void
ReaderContext< SampleType >::deactivate(CoreKit::RunLoop *runLoop)
{
    m_reader.listener(nullptr, dds::core::status::StatusMask::none());
    runLoop->deregisterInputSource(&m_readerMux);
}


template< typename SampleType >
void
ReaderContext< SampleType >::inputAvailableFrom(CoreKit::InputSource *source)
{
    if (source == &m_readerMux)
    {
        m_readerMux.dispatchDeferred();
    }
}


template< typename SampleType >
typename ReaderContext< SampleType >::SampleDistribution&
ReaderContext< SampleType >::sampleDistribution()
{
    if (!m_sampleDistribution)
    {
        m_sampleDistribution = std::make_shared< SampleDistribution >();
        m_readerMux.newReaderDataActionFac = std::make_shared< typename EmitNewReaderDataAction< SampleType >::FactoryCall >(m_sampleDistribution);
    }

    return *m_sampleDistribution;
}


template< typename SampleType >
SampleLostStatusDistribution&
ReaderContext< SampleType >::sampleLostStatusDistribution()
{
    if (!m_sampleLostStatusDistribution)
    {
        m_sampleLostStatusDistribution = std::make_shared< SampleLostStatusDistribution >();
        m_readerMux.sampleLostStatusActionFac = std::make_shared< EmitSampleLostStatusAction::FactoryCall >(m_sampleLostStatusDistribution);
    }

    return *m_sampleLostStatusDistribution;
}


template< typename SampleType >
SubscriptionMatchedStatusDistribution&
ReaderContext< SampleType >::subscriptionMatchedStatusDistribution()
{
    if (!m_subscriptionMatchedStatusDistribution)
    {
        m_subscriptionMatchedStatusDistribution = std::make_shared< SubscriptionMatchedStatusDistribution >();
        m_readerMux.subscriptionMatchedStatusActionFac = std::make_shared< EmitSubscriptionMatchedStatusAction::FactoryCall >(m_subscriptionMatchedStatusDistribution);
    }

    return *m_subscriptionMatchedStatusDistribution;
}

} // end namespace IsoDdsKit

#endif /* !_FOUNDATION_ISODDSKIT_READERCONTEXT_HPP_ */

// vim: set ts=4 sw=4 expandtab:
