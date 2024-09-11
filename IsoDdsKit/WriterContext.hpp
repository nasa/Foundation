/**
 * \file IsoDdsKit/WriterContext.hpp
 * \brief Contains the definition of the \c IsoDdsKit::WriterContext class.
 * \author Rolando J. Nieves
 * \date 2024-08-23 15:35:38
 */

#ifndef _FOUNDATION_ISODDSKIT_WRITERCONTEXT_HPP_
#define _FOUNDATION_ISODDSKIT_WRITERCONTEXT_HPP_

#include <CoreKit/CoreKit.h>

#include <IsoDdsKit/WriterStimuliMux.hpp>
#include <IsoDdsKit/status_types.hpp>


namespace IsoDdsKit
{

template< typename SampleType >
class WriterContext : public CoreKit::InterruptListener
{
private:
    using Writer = dds::pub::DataWriter< SampleType >;
    using PublicationMatchedStatusDistributionPtr = std::shared_ptr< PublicationMatchedStatusDistribution >;
    using WriterMux = WriterStimuliMux< SampleType >;

    Writer m_writer;
    WriterMux m_writerMux;
    PublicationMatchedStatusDistributionPtr m_publicationMatchedStatusDistribution;

public:
    explicit WriterContext(dds::pub::Publisher& publisher, dds::topic::Topic< SampleType >& topic);

    explicit WriterContext(dds::pub::Publisher& publisher, dds::topic::Topic< SampleType >& topic, dds::pub::qos::DataWriterQos const& qos);

    WriterContext(WriterContext const& other) = delete;
    WriterContext(WriterContext&& other) = delete;

    virtual ~WriterContext();

    void activate(CoreKit::RunLoop *runLoop);

    void deactivate(CoreKit::RunLoop *runLoop);

    virtual void inputAvailableFrom(CoreKit::InputSource *source) override;

    inline Writer& writer()
    { return m_writer; }

    PublicationMatchedStatusDistribution& publicationMatchedStatusDistribution();
};


template< typename SampleType >
WriterContext< SampleType >::WriterContext(
    dds::pub::Publisher& publisher,
    dds::topic::Topic< SampleType >& topic
):
    m_writer(dds::core::null),
    m_writerMux(this)
{
    m_writer = Writer(publisher, topic);
}


template< typename SampleType >
WriterContext< SampleType >::WriterContext(
    dds::pub::Publisher& publisher,
    dds::topic::Topic< SampleType >& topic,
    dds::pub::qos::DataWriterQos const& qos
):
    m_writer(dds::core::null),
    m_writerMux(this)
{
    m_writer = Writer(publisher, topic, qos);
}


template< typename SampleType >
WriterContext< SampleType >::~WriterContext()
{
    m_writer.close();
}


template< typename SampleType >
void
WriterContext< SampleType >::activate(CoreKit::RunLoop *runLoop)
{
    runLoop->registerInputSource(&m_writerMux);
    m_writer.listener(&m_writerMux, m_writerMux.statusMask());
}


template< typename SampleType >
void
WriterContext< SampleType >::deactivate(CoreKit::RunLoop *runLoop)
{
    m_writer.listener(nullptr, dds::core::status::StatusMask::none());
    runLoop->deregisterInputSource(&m_writerMux);
}

template< typename SampleType >
void
WriterContext< SampleType >::inputAvailableFrom(CoreKit::InputSource *source)
{
    if (source == &m_writerMux)
    {
        m_writerMux.dispatchDeferred();
    }
}


template< typename SampleType >
PublicationMatchedStatusDistribution&
WriterContext< SampleType >::publicationMatchedStatusDistribution()
{
    if (!m_publicationMatchedStatusDistribution)
    {
        m_publicationMatchedStatusDistribution = std::make_shared< PublicationMatchedStatusDistribution >();
        m_writerMux.publicationMatchedStatusActionFac = std::make_shared< EmitPublicationMatchedStatusAction::FactoryCall >(m_publicationMatchedStatusDistribution);
    }

    return *m_publicationMatchedStatusDistribution;
}

} // end namespace IsoDdsKit

#endif /* !_FOUNDATION_ISODDSKIT_WRITERCONTEXT_HPP_ */

// vim: set ts=4 sw=4 expandtab:
