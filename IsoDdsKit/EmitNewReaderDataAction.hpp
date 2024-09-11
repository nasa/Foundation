/**
 * \file IsoDdsKit/EmitNewReaderDataAction.hpp
 * \brief Contains the definition of the \c IsoDdsKit::EmitNewReaderDataAction class.
 * \author Rolando J. Nieves
 * \date 2024-08-22 13:45:46
 */

#ifndef _FOUNDATION_ISODDSKIT_EMITNEWREADERDATACTION_HPP_
#define _FOUNDATION_ISODDSKIT_EMITNEWREADERDATACTION_HPP_

#include <memory>
#include <stdexcept>

#include <dds/sub/DataReader.hpp>

#include <IsoDdsKit/ValueDistribution.hpp>


namespace IsoDdsKit
{

/**
 * \brief Closure that polls a reader for samples.
 *
 * Instances of this class are used to store all information necessary to poll
 * a \c dds::sub::DataReader for available samples. The context for the sample
 * poll includes the reader instance and the event distributor used to
 * communicate samples to interested listeners. The class is primarily used to
 * create closures that can be deferred for later execution, usually to
 * coordinate between a DDS provider's listener thread and the application main
 * thread.
 *
 * The class definition also includes an inner class that can be used to
 * fabricate instances with part of the closure context already established.
 */
template< typename SampleType >
class EmitNewReaderDataAction
{
public:
    /**
     * \brief Type alias for the data reader polled.
     */
    using Reader = dds::sub::DataReader< SampleType >;

    /**
     * \brief Type alias for a shared pointer to the event distributor.
     */
    using SampleDistributionPtr = std::shared_ptr< ValueDistribution< SampleType > >;

private:
    Reader m_reader;
    SampleDistributionPtr m_sampleDistribution;

public:
    /**
     * \brief Factory for creating reader sample poll closures.
     *
     * This factory class serves as a partial binding for the data reader poll
     * closure, by remembering the event distributor to associate with the
     * created closures.
     */
    class FactoryCall
    {
    private:
        SampleDistributionPtr m_sampleDistribution;
    
    public:
        /**
         * \brief Remember the event distributor to use when creating closures.
         *
         * \param sampleDistribution Event distributor to bind to closures.
         */
        explicit FactoryCall(SampleDistributionPtr sampleDistribution);

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
         * \brief Create new data reader poll closure.
         *
         * This method combines the event distributor provided at construction
         * time with the reader instance provided in the argument list to
         * create a new data reader poll closure.
         *
         * \param reader Data reader that will be polled in the created
         *        closure.
         *
         * \return New data reader poll closure.
         */
        EmitNewReaderDataAction< SampleType > createNew(Reader& reader);
    };

    /**
     * \brief Create the data reader polling closure.
     *
     * References to both the reader and the event distributor are held through
     * the lifetime of this closure object.
     *
     * \param reader Data reader to poll.
     * \param sampleDistribution Event distributor to use.
     */
    explicit EmitNewReaderDataAction(Reader& reader, SampleDistributionPtr sampleDistribution);

    /**
     * \brief Move constructor.
     *
     * This constructor is required to make these closures compatible with
     * \c std::future<>
     */
    EmitNewReaderDataAction(EmitNewReaderDataAction&& other);

    EmitNewReaderDataAction(EmitNewReaderDataAction const& other) = delete;

    /**
     * \brief Default destructor.
     */
    virtual ~EmitNewReaderDataAction() = default;

    /**
     * \brief Poll data reader for samples and distribute them.
     */
    void operator()();
};

template< typename SampleType >
EmitNewReaderDataAction< SampleType >::FactoryCall::FactoryCall(SampleDistributionPtr sampleDistribution):
    m_sampleDistribution(sampleDistribution)
{
    if (!m_sampleDistribution)
    {
        throw std::logic_error("EmitNewReaderDataAction::FactoryCall: SampleDistribution is null.");
    }
}

template< typename SampleType >
EmitNewReaderDataAction< SampleType >::FactoryCall::FactoryCall(FactoryCall const& other):
    m_sampleDistribution(other.m_sampleDistribution)
{}

template< typename SampleType >
EmitNewReaderDataAction< SampleType >::FactoryCall::FactoryCall(FactoryCall&& other):
    m_sampleDistribution(std::move(other.m_sampleDistribution))
{}

template< typename SampleType >
EmitNewReaderDataAction< SampleType >
EmitNewReaderDataAction< SampleType >::FactoryCall::createNew(Reader& reader)
{
    return EmitNewReaderDataAction< SampleType >(reader, m_sampleDistribution);
}

template< typename SampleType >
EmitNewReaderDataAction< SampleType >::EmitNewReaderDataAction(Reader& reader, SampleDistributionPtr sampleDistribution):
    m_reader(reader),
    m_sampleDistribution(sampleDistribution)
{}

template< typename SampleType >
EmitNewReaderDataAction< SampleType >::EmitNewReaderDataAction(EmitNewReaderDataAction< SampleType >&& other):
    m_reader(other.m_reader),
    m_sampleDistribution(other.m_sampleDistribution)
{}

template< typename SampleType >
void
EmitNewReaderDataAction< SampleType >::operator()()
{
    if (!m_sampleDistribution || m_reader.is_nil())
    {
        throw std::runtime_error("EmitNewReaderDataAction: Reader or SampleDistribution null.");
    }

    auto samples = m_reader.select().max_samples(10).state(dds::sub::status::DataState::new_data()).take();
    for (auto const& aSample : samples)
    {
        if (aSample.info().valid())
        {
            m_sampleDistribution->emit(aSample.data());
        }
    }
}

} // end namespace IsoDdsKit

#endif /* !_FOUNDATION_ISODDSKIT_EMITNEWREADERDATACTION_HPP_ */

// vim: set ts=4 sw=4 expandtab:
