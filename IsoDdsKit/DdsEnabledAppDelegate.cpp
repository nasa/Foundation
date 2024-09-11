/**
 * \file IsoDdsKit/DdsEnabledAppDelegate.cpp
 * \brief Contains the implementation of the \c IsoDdsKit::DdsEnabledAppDelegate class.
 * \author Rolando J. Nieves
 * \date 2024-08-23 13:07:13
 */

#include "DdsEnabledAppDelegate.hpp"


using CoreKit::Application;

namespace
{

const char *DDS_DOMAIN_FLAG = "dds-domain";

} // end anonymous namespace

namespace IsoDdsKit
{

DdsEnabledAppDelegate::DdsEnabledAppDelegate():
    m_ddsDomainId(-1),
    m_participant(dds::core::null),
    m_publisher(dds::core::null),
    m_subscriber(dds::core::null)
{}


void
DdsEnabledAppDelegate::configureCmdLineArgs(Application *theApp)
{
    theApp->addCmdLineArgDef(
        Application::CmdLineArg(
            DDS_DOMAIN_FLAG,
            true,
            "Identifier of domain to participate in."
        )
    );
}


void
DdsEnabledAppDelegate::applicationDidInitialize(Application *theApp)
{
    m_ddsDomainId = std::stoi(theApp->getCmdLineArgFor(DDS_DOMAIN_FLAG));
}


void
DdsEnabledAppDelegate::applicationDidTerminate(Application *theApp)
{
    m_topics.clear();
    if (!m_subscriber.is_nil())
    {
        m_subscriber.close();
        m_subscriber = dds::core::null;
    }
    if (!m_publisher.is_nil())
    {
        m_publisher.close();
        m_publisher = dds::core::null;
    }
    if (!m_participant.is_nil())
    {
        m_participant.close();
        m_participant = dds::core::null;
    }
}


dds::domain::DomainParticipant&
DdsEnabledAppDelegate::participant()
{
    if (m_participant.is_nil())
    {
        auto qos = dds::domain::DomainParticipant::default_participant_qos();
        return participant(qos);
    }
    return m_participant;
}


dds::domain::DomainParticipant&
DdsEnabledAppDelegate::participant(dds::domain::qos::DomainParticipantQos const& qos)
{
    if (!m_participant.is_nil())
    {
        throw std::logic_error("Cannot use participant() QoS overload with already-existing participant.");
    }
    m_participant = dds::domain::DomainParticipant(m_ddsDomainId, qos);

    return m_participant;
}


dds::pub::Publisher&
DdsEnabledAppDelegate::publisher()
{
    if (m_publisher.is_nil())
    {
        auto qos = participant().default_publisher_qos();
        return publisher(qos);
    }

    return m_publisher;
}


dds::pub::Publisher&
DdsEnabledAppDelegate::publisher(dds::pub::qos::PublisherQos const& qos)
{
    if (!m_publisher.is_nil())
    {
        throw std::logic_error("Cannot use publisher() QoS overload with already-existing publisher.");
    }
    m_publisher = dds::pub::Publisher(participant(), qos);

    return m_publisher;
}


dds::sub::Subscriber&
DdsEnabledAppDelegate::subscriber()
{
    if (m_subscriber.is_nil())
    {
        auto qos = participant().default_subscriber_qos();
        return subscriber(qos);
    }

    return m_subscriber;
}


dds::sub::Subscriber&
DdsEnabledAppDelegate::subscriber(dds::sub::qos::SubscriberQos const& qos)
{
    if (!m_subscriber.is_nil())
    {
        throw std::logic_error("Cannot use subscriber() QoS overload with already-existing subscriber.");
    }
    m_subscriber = dds::sub::Subscriber(participant(), qos);

    return m_subscriber;
}

} // end namespace IsoDdsKit

// vim: set ts=4 sw=4 expandtab:
