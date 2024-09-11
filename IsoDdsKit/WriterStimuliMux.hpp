/**
 * \file IsoDdsKit/WriterStimuliMux.hpp
 * \brief Contains the definition of the \c IsoDdsKit::WriterStimuliMux class.
 * \author Rolando J. Nieves
 * \date 2024-08-23 14:37:50
 */

#ifndef _FOUNDATION_ISODDSKIT_WRITERSTIMULIMUX_HPP_
#define _FOUNDATION_ISODDSKIT_WRITERSTIMULIMUX_HPP_

#include <memory>

#include <dds/core/status/State.hpp>
#include <dds/pub/DataWriter.hpp>
#include <dds/pub/DataWriterListener.hpp>

#include <CoreKit/CoreKit.h>

#include <IsoDdsKit/DeferredActions.hpp>
#include <IsoDdsKit/EmitStatusAction.hpp>
#include <IsoDdsKit/status_types.hpp>


namespace IsoDdsKit
{

template< typename SampleType >
class WriterStimuliMux : public dds::pub::NoOpDataWriterListener< SampleType >, public CoreKit::EventInputSource
{
private:
    DeferredActions m_deferredActions;

public:
    using Writer = dds::pub::DataWriter< SampleType >;
    using PublicationMatchedStatusActionFactoryPtr = std::shared_ptr< EmitPublicationMatchedStatusAction::FactoryCall >;

    PublicationMatchedStatusActionFactoryPtr publicationMatchedStatusActionFac;

    explicit WriterStimuliMux(CoreKit::InterruptListener *listener);

    WriterStimuliMux(WriterStimuliMux const& other) = delete;
    WriterStimuliMux(WriterStimuliMux&& other) = delete;

    virtual ~WriterStimuliMux() = default;

    void dispatchDeferred();

    virtual void on_publication_matched(dds::pub::DataWriter< SampleType >& writer, dds::core::status::PublicationMatchedStatus const& status) override;

    dds::core::status::StatusMask statusMask() const;

    WriterStimuliMux& operator=(WriterStimuliMux const& other) = delete;
    WriterStimuliMux& operator=(WriterStimuliMux&& other) = delete;
};

template< typename SampleType >
WriterStimuliMux< SampleType >::WriterStimuliMux(CoreKit::InterruptListener *listener):
    CoreKit::EventInputSource(listener)
{}

template< typename SampleType >
void
WriterStimuliMux< SampleType >::dispatchDeferred()
{
    m_deferredActions.dispatchAll();
}

template< typename SampleType >
void
WriterStimuliMux< SampleType >::on_publication_matched(dds::pub::DataWriter< SampleType >& writer, dds::core::status::PublicationMatchedStatus const& status)
{
    if (publicationMatchedStatusActionFac)
    {
        m_deferredActions.add(
            std::async(
                std::launch::deferred,
                std::move(publicationMatchedStatusActionFac->createNew(status))
            )
        );
    }
    this->assertEvent();
}

template< typename SampleType >
dds::core::status::StatusMask
WriterStimuliMux< SampleType >::statusMask() const
{
    dds::core::status::StatusMask result = dds::core::status::StatusMask::none();

    if (publicationMatchedStatusActionFac)
    {
        result |= dds::core::status::StatusMask::publication_matched();
    }

    return result;
}

} // end namespace IsoDdsKit

#endif /* !_FOUNDATION_ISODDSKIT_WRITERSTIMULIMUX_HPP_ */

// vim: set ts=4 sw=4 expandtab:
