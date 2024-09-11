/**
 * \file IsoDdsKit/ReaderStimuliMux.hpp
 * \brief Contains the definition of the \c IsoDdsKit::ReaderStimulusMux class.
 * \author Rolando J. Nieves
 * \date 2024-08-22 15:39:54
 */

#ifndef _FOUNDATION_ISODDSKIT_READERSTIMULIMUX_HPP_
#define _FOUNDATION_ISODDSKIT_READERSTIMULIMUX_HPP_

#include <future>
#include <memory>
#include <mutex>
#include <deque>

#include <dds/core/status/State.hpp>
#include <dds/sub/DataReader.hpp>
#include <dds/sub/DataReaderListener.hpp>

#include <CoreKit/CoreKit.h>

#include <IsoDdsKit/DeferredActions.hpp>
#include <IsoDdsKit/EmitNewReaderDataAction.hpp>
#include <IsoDdsKit/status_types.hpp>


namespace IsoDdsKit
{

template< typename SampleType >
class ReaderStimuliMux : public dds::sub::NoOpDataReaderListener< SampleType >, public CoreKit::EventInputSource
{
private:
    DeferredActions m_deferredActions;

public:
    using Reader = dds::sub::DataReader< SampleType >;
    using NewReaderDataActionFactoryPtr = std::shared_ptr< typename EmitNewReaderDataAction< SampleType >::FactoryCall >;
    using SampleLostStatusActionFactoryPtr = std::shared_ptr< EmitSampleLostStatusAction::FactoryCall >;
    using SubscriptionMatchedStatusActionFactoryPtr = std::shared_ptr< EmitSubscriptionMatchedStatusAction::FactoryCall >;

    NewReaderDataActionFactoryPtr newReaderDataActionFac;
    SampleLostStatusActionFactoryPtr sampleLostStatusActionFac;
    SubscriptionMatchedStatusActionFactoryPtr subscriptionMatchedStatusActionFac;

    explicit ReaderStimuliMux(CoreKit::InterruptListener *listener);

    ReaderStimuliMux(ReaderStimuliMux const& other) = delete;
    ReaderStimuliMux(ReaderStimuliMux&& other) = delete;

    virtual ~ReaderStimuliMux() = default;

    void dispatchDeferred();

    virtual void on_data_available(Reader& reader) override;

    virtual void on_sample_lost(Reader& reader, dds::core::status::SampleLostStatus const& status) override;

    virtual void on_subscription_matched(Reader& reader, dds::core::status::SubscriptionMatchedStatus const& status) override;

    dds::core::status::StatusMask statusMask() const;

    ReaderStimuliMux& operator=(ReaderStimuliMux const& other) = delete;
    ReaderStimuliMux& operator=(ReaderStimuliMux&& other) = delete;
};

template< typename SampleType >
ReaderStimuliMux< SampleType >::ReaderStimuliMux(CoreKit::InterruptListener *listener):
    CoreKit::EventInputSource(listener)
{}

template< typename SampleType >
void
ReaderStimuliMux< SampleType >::dispatchDeferred()
{
    m_deferredActions.dispatchAll();
}

template< typename SampleType >
void
ReaderStimuliMux< SampleType >::on_data_available(Reader& reader)
{
    if (newReaderDataActionFac)
    {
        m_deferredActions.add(
            std::async(
                std::launch::deferred,
                std::move(newReaderDataActionFac->createNew(reader))
            )
        );
        this->assertEvent();
    }
}

template< typename SampleType >
void
ReaderStimuliMux< SampleType >::on_sample_lost(Reader& reader, dds::core::status::SampleLostStatus const& status)
{
    if (sampleLostStatusActionFac)
    {
        m_deferredActions.add(
            std::async(
                std::launch::deferred,
                std::move(sampleLostStatusActionFac->createNew(status))
            )
        );
        this->assertEvent();
    }
}

template< typename SampleType >
void
ReaderStimuliMux< SampleType >::on_subscription_matched(Reader& reader, dds::core::status::SubscriptionMatchedStatus const& status)
{
    if (subscriptionMatchedStatusActionFac)
    {
        m_deferredActions.add(
            std::async(
                std::launch::deferred,
                std::move(subscriptionMatchedStatusActionFac->createNew(status))
            )
        );
        this->assertEvent();
    }
}

template< typename SampleType >
dds::core::status::StatusMask
ReaderStimuliMux< SampleType >::statusMask() const
{
    dds::core::status::StatusMask result = dds::core::status::StatusMask::none();

    if (newReaderDataActionFac)
    {
        result |= dds::core::status::StatusMask::data_available();
    }

    if (sampleLostStatusActionFac)
    {
        result |= dds::core::status::StatusMask::sample_lost();
    }

    if (subscriptionMatchedStatusActionFac)
    {
        result |= dds::core::status::StatusMask::subscription_matched();
    }

    return result;
}

} // end namespace IsoDdsKit

#endif /* !_FOUNDATION_ISODDSKIT_READERSTIMULIMUX_HPP_ */

// vim: set ts=4 sw=4 expandtab:
