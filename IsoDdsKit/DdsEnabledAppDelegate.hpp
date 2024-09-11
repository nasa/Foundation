/**
 * \file IsoDdsKit/DdsEnabledAppDelegate.hpp
 * \brief Contains the definition of the \c IsoDdsKit::DdsEnabledAppDelegate class.
 * \author Rolando J. Nieves
 * \date 2024-08-23 11:43:27
 */

#ifndef _FOUNDATION_ISODDSKIT_DDSENABLEDAPPDELEGATE_HPP_
#define _FOUNDATION_ISODDSKIT_DDSENABLEDAPPDELEGATE_HPP_

#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

#include <dds/domain/DomainParticipant.hpp>
#include <dds/sub/ddssub.hpp>
#include <dds/pub/ddspub.hpp>

#include <CoreKit/CoreKit.h>

#include <IsoDdsKit/ReaderContext.hpp>
#include <IsoDdsKit/TopicCollection.hpp>
#include <IsoDdsKit/WriterContext.hpp>


namespace IsoDdsKit
{

/**
 * \brief Base application delegate with basic DDS functionality.
 *
 * The \c IsoDdsKit::DdsEnabledAppDelegate is offered as an option to
 * developers writing Foundation-based, DDS-enabled applications. The class on
 * its own only maintains DDS infrastructure. It is meant to be used as a base
 * class by application developers.
 *
 * \note Use of this delegate base class is \e not required from DDS-enabled,
 *       Foundation applications.
 */
class DdsEnabledAppDelegate : public CoreKit::AppDelegate
{
private:
    int m_ddsDomainId;
    dds::domain::DomainParticipant m_participant;
    dds::pub::Publisher m_publisher;
    dds::sub::Subscriber m_subscriber;
    TopicCollection m_topics;

public:
    /**
     * \brief Type alias for data reader context shared pointers.
     */
    template< typename SampleType > using TypedReaderContextPtr = std::shared_ptr< ReaderContext< SampleType > >;

    /**
     * \brief Type alias for data writer context shared pointers.
     */
    template< typename SampleType > using TypedWriterContextPtr = std::shared_ptr< WriterContext< SampleType > >;

    /**
     * \brief Initialize DDS entity fields to nominal values.
     *
     * The entity fields are set to \c dds::core::null .
     */
    DdsEnabledAppDelegate();

    /**
     * \brief Default destructor.
     */
    virtual ~DdsEnabledAppDelegate() = default;

    /**
     * \brief Add DDS-specific command-line arguments.
     *
     * The \c configureCmdLineArgs() method configures the following
     * command-line flags:
     * * \c --dds-domain : Used to specify the identifier for the DDS domain
     *   the application should participate in.
     *
     * \param theApp - \c CoreKit::Application instance to use when configuring
     *        the command line argument.
     */
    virtual void configureCmdLineArgs(CoreKit::Application *theApp);

    /**
     * \brief Read in and process command-line configuration.
     *
     * The \c applicationDidInitialize() method parses the information provided
     * by users via the command line and preserves the information for future
     * use.
     *
     * \param theApp - \c CoreKit::Application instance to use when acquiring
     *        command-line arguments.
     */
    virtual void applicationDidInitialize(CoreKit::Application *theApp) override;

    /**
     * \brief Tear down all previously-created DDS infrastructure.
     *
     * The \c applicationDidTerminate() method will destroy (or "close out")
     * any DDS infrastructure (Participant, Subscriber, Publisher) created
     * during the application lifetime.
     *
     * \param theApp - Associated \c CoreKit::Application instance; not used in
     *        this method.
     */
    virtual void applicationDidTerminate(CoreKit::Application *theApp) override;

    /**
     * \brief Access the \c dds::domain::DomainParticipant this instance manages.
     *
     * The \c participant() method provides application developers with access
     * to the \c dds::domain::DomainParticipant instance managed by this class.
     * This method lazily creates the \c dds::domain::DomainParticipant
     * instance if it had not been created prior to this method being called.
     * 
     *
     * \return \c dds::domain::DomainParticipant instance.
     */
    dds::domain::DomainParticipant& participant();

    /**
     * \brief Create a \c dds::domain::DomainParticipant instance to manage.
     *
     * This overload of the \c participant() method creates a
     * \c dds::domain::DomainParticipant instance using the provided QoS and
     * returns it to the caller. If a participant instance had already been
     * created prior to a call to this method overload, an exception is raised.
     *
     * \param qos Quality of service instance to use when creating participant.
     *
     * \return \c dds::domain::DomainParticipant instance.
     *
     * \throws \c std::logic_error if a participant instance had already been
     *         created.
     */
    dds::domain::DomainParticipant& participant(
        dds::domain::qos::DomainParticipantQos const& qos
    );

    /**
     * \brief Access the \c dds::pub::Publisher this instance manages.
     *
     * The \c publisher() method provides application developers with access
     * to the \c dds::pub::Publisher instance managed by this class.
     * This method lazily creates the \c dds::pub::Publisher instance if it
     * had not been created prior to the method being called.
     *
     * \return \c dds::pub::Publisher instance.
     */
    dds::pub::Publisher& publisher();

    /**
     * \brief Create a \c dds::pub::Publisher instance to manage.
     *
     * This overload of the \c publisher() method creates a
     * \c dds::pub::Publisher instance using the provided QoS and
     * returns it to the caller. If a publisher instance had already been
     * created prior to a call to this method overload, an exception is raised.
     *
     * \param qos Quality of service instance to use when creating the
     *        publisher.
     *
     * \return \c dds::pub::Publisher instance.
     *
     * \throws \c std::logic_error if a publisher instance had already been
     *         created.
     */
    dds::pub::Publisher& publisher(dds::pub::qos::PublisherQos const& qos);

    /**
     * \brief Create a new context for \c dds::sub::DataReader instance.
     *
     * This variant of the \c createReaderContext() method uses the required
     * infrastructure managed by this class to create a reader context object
     * The reader context instance integrates a data reader into the
     * Foundation run loop. The call creates the new \c dds::sub::DataReader
     * using the default QoS settings.
     *
     * \note This method does not lazily create the required infrastructure,
     *       such as the domain participant or the subscriber. If this method
     *       is called before those are available, an exception is thrown.
     *
     * \param topicName Name of the topic the created data reader will use.
     *
     * \return Shared pointer to the \c IsoDdsKit::ReaderContext instance
     *         created.
     *
     * \throws \c std::logic_error if the required infrastructure is not
     *         present when the method is called.
     */
    template< typename SampleType >
    TypedReaderContextPtr< SampleType > createReaderContext(
        std::string const& topicName
    );

    /**
     * \brief Create a new context for \c dds::sub::DataReader instance.
     *
     * This variant of the \c createReaderContext() method uses the required
     * infrastructure managed by this class to create a reader context object
     * The reader context instance integrates a data reader into the
     * Foundation run loop. The call creates the new \c dds::sub::DataReader
     * using the provided QoS settings.
     *
     * \note This method does not lazily create the required infrastructure,
     *       such as the domain participant or the subscriber. If this method
     *       is called before those are available, an exception is thrown.
     *
     * \param topicName Name of the topic the created data reader will use.
     * \param qos Quality of service settings to use when creating the data
     *        reader.
     *
     * \return Shared pointer to the \c IsoDdsKit::ReaderContext instance
     *         created.
     *
     * \throws \c std::logic_error if the required infrastructure is not
     *         present when the method is called.
     */
    template< typename SampleType >
    TypedReaderContextPtr< SampleType > createReaderContext(
        std::string const& topicName,
        dds::sub::qos::DataReaderQos const& qos
    );

    /**
     * \brief Create a new context for \c dds::pub::DataWriter instance.
     *
     * This variant of the \c createWriterContext() method uses the required
     * infrastructure managed by this class to create a writer context object
     * The writer context instance integrates a data writer into the
     * Foundation run loop. The call creates the new \c dds::pub::DataWriter
     * using the default QoS settings.
     *
     * \note This method does not lazily create the required infrastructure,
     *       such as the domain participant or the publisher. If this method
     *       is called before those are available, an exception is thrown.
     *
     * \param topicName Name of the topic the created data writer will use.
     *
     * \return Shared pointer to the \c IsoDdsKit::WriterContext instance
     *         created.
     *
     * \throws \c std::logic_error if the required infrastructure is not
     *         present when the method is called.
     */
    template< typename SampleType >
    TypedWriterContextPtr< SampleType > createWriterContext(
        std::string const& topicName
    );

    /**
     * \brief Create a new context for \c dds::pub::DataWriter instance.
     *
     * This variant of the \c createWriterContext() method uses the required
     * infrastructure managed by this class to create a writer context object
     * The writer context instance integrates a data writer into the
     * Foundation run loop. The call creates the new \c dds::pub::DataWriter
     * using the provided QoS settings.
     *
     * \note This method does not lazily create the required infrastructure,
     *       such as the domain participant or the publisher. If this method
     *       is called before those are available, an exception is thrown.
     *
     * \param topicName Name of the topic the created data writer will use.
     * \param qos Quality of service settings to use when creating the data
     *        writer.
     *
     * \return Shared pointer to the \c IsoDdsKit::WriterContext instance
     *         created.
     *
     * \throws \c std::logic_error if the required infrastructure is not
     *         present when the method is called.
     */
    template< typename SampleType >
    TypedWriterContextPtr< SampleType > createWriterContext(
        std::string const& topicName,
        dds::pub::qos::DataWriterQos const& qos
    );

    /**
     * \brief Access the \c dds::sub::Subscriber this instance manages.
     *
     * The \c subscriber() method provides application developers with access
     * to the \c dds::sub::Subscriber instance managed by this class.
     * This method lazily creates the \c dds::sub::Subscriber instance if it
     * had not been created prior to the method being called.
     *
     * \return \c dds::sub::Subscriber instance.
     */
    dds::sub::Subscriber& subscriber();

    /**
     * \brief Create a \c dds::sub::Subscriber instance to manage.
     *
     * This overload of the \c subscriber() method creates a
     * \c dds::sub::Subscriber instance using the provided QoS and
     * returns it to the caller. If a subscriber instance had already been
     * created prior to a call to this method overload, an exception is raised.
     *
     * \param qos Quality of service instance to use when creating the
     *        subscriber.
     *
     * \return \c dds::sub::Subscriber instance.
     *
     * \throws \c std::logic_error if a subscriber instance had already been
     *         created.
     */
    dds::sub::Subscriber& subscriber(dds::sub::qos::SubscriberQos const& qos);

    /**
     * \brief Access the topic management subordinate.
     *
     * The \c topics() method provides access to the
     * \c IsoDdsKit::TopicCollection instance managed by this object. This
     * instance uses information stored in this topic manager when creating
     * reader and writer contexts.
     *
     * \see \c IsoDdsKit::DdsEnabledAppDelegate::createReaderContext()
     * \see \c IsoDdsKit::DdsEnabledAppDelegate::createWriterContext()
     *
     * \return \c IsoDdsKit::TopicCollection instance used by this instance.
     */
    inline TopicCollection& topics() { return m_topics; }
};


template< typename SampleType >
DdsEnabledAppDelegate::TypedReaderContextPtr< SampleType >
DdsEnabledAppDelegate::createReaderContext(std::string const& topicName)
{
    auto defaultQos = m_subscriber.default_datareader_qos();
    return this->createReaderContext< SampleType >(topicName, defaultQos);
}


template< typename SampleType >
DdsEnabledAppDelegate::TypedReaderContextPtr< SampleType >
DdsEnabledAppDelegate::createReaderContext(std::string const& topicName, dds::sub::qos::DataReaderQos const& qos)
{
    std::stringstream errorMsg;

    if (m_subscriber.is_nil())
    {
        throw std::logic_error("Cannot create ReaderContext without Subscriber.");
    }
    dds::topic::Topic< SampleType > topic = m_topics.use< SampleType >(topicName);
    if (topic.is_nil())
    {
        errorMsg << "Cannot create ReaderContext for unknown Topic \"" << topicName << "\"";
        throw std::logic_error(errorMsg.str());
    }
    return std::make_shared< ReaderContext< SampleType > >(
        m_subscriber,
        topic,
        qos
    );
}


template< typename SampleType >
DdsEnabledAppDelegate::TypedWriterContextPtr< SampleType >
DdsEnabledAppDelegate::createWriterContext(std::string const& topicName)
{
    auto defaultQos = m_subscriber.default_datareader_qos();
    return this->createWriterContext< SampleType >(topicName, defaultQos);
}


template< typename SampleType >
DdsEnabledAppDelegate::TypedWriterContextPtr< SampleType >
DdsEnabledAppDelegate::createWriterContext(std::string const& topicName, dds::pub::qos::DataWriterQos const& qos)
{
    std::stringstream errorMsg;

    if (m_publisher.is_nil())
    {
        throw std::logic_error("Cannot create WriterContext without Publisher.");
    }
    dds::topic::Topic< SampleType > topic = m_topics.use< SampleType >(topicName);
    if (topic.is_nil())
    {
        errorMsg << "Cannot create WriterContext for unknown Topic \"" << topicName << "\"";
        throw std::logic_error(errorMsg.str());
    }
    return std::make_shared< WriterContext< SampleType > >(
        m_publisher,
        topic,
        qos
    );
}

} // end namespace IsoDdsKit

#endif /* !_FOUNDATION_ISODDSKIT_DDSENABLEDAPPDELEGATE_HPP_ */

// vim: set ts=4 sw=4 expandtab:
