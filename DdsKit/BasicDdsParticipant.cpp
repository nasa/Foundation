/**
 * \file BasicDdsParticipant.cpp
 * \brief Implementation of the BasicDdsParticipant Class
 * \date 2012-09-26 17:11:17
 * \author Rolando J. Nieves
 */

#include <algorithm>
#include <functional>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>

#include <CoreKit/PreconditionNotMetException.h>
#include <CoreKit/InvalidInputException.h>
#include <CoreKit/factory.h>
#include <CoreKit/CmdLineMultiArg.h>

#include <DdsErrorException.h>

#include "BasicDdsParticipant.h"

using std::for_each;
using std::ptr_fun;
using std::string;
using std::vector;
using DDS::DomainId_t;
using DDS::DomainParticipantQos;
using DDS::DomainParticipantFactory;
using DDS::DomainParticipant;
using DDS::Publisher;
using DDS::Subscriber;
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
using DDS::PUBLISHER_QOS_DEFAULT;
using DDS::SUBSCRIBER_QOS_DEFAULT;
using DDS::ParticipantLocator;
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
using DDS::DomainParticipantFactory_var;
using DDS::DomainParticipantListener_ptr;
#endif /* DDS_PROVIDER */
using CoreKit::InvalidInputException;
using CoreKit::PreconditionNotMetException;
using CoreKit::destroy;
using CoreKit::AppLog;
using CoreKit::EndLog;
using CoreKit::CmdLineMultiArg;
using CoreKit::Application;

namespace DdsKit {

/**
 * \brief Constant used to denote an invalid DDS Domain ID value.
 * Per the RTPS specification OMG Document number (formal/2009-01-05), section
 * 9.6.1.3, a DDS domain ID value is limited in range from 0 to 229 in the UDP
 * platform specific model, which is the model that we rely on here.
 */
const DomainId_t INVALID_DOMAIN = 230u;
const string BasicDdsParticipant::DDS_PROVIDER_ARG_FLAG = "dpa";

void BasicDdsParticipant::AddDdsRelatedCmdArgs(Application *theApp)
{
	theApp->addCmdLineArgDef(Application::CmdLineArg(DDS_PROVIDER_ARG_FLAG, true, "DDS provider-specific command line to pass on."));
}

vector< string > BasicDdsParticipant::GatherDdsRelatedCmdArgs(Application *theApp)
{
	string argVal;

	argVal = theApp->getCmdLineArgFor(DDS_PROVIDER_ARG_FLAG);
	CmdLineMultiArg dpaArgs(argVal);

	return dpaArgs.argList;
}

BasicDdsParticipant::BasicDdsParticipant(DDS::DomainId_t domainId, AppLog *log)
: m_participantName("BasicDdsParticipant"),
  m_log(log),
  m_peerMonitoring(NULL),
  m_domainId(domainId),
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
  m_appParticipant(NULL),
  m_publisher(NULL),
  m_subscriber(NULL)
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
  m_appParticipant(DomainParticipant::_nil()),
  m_publisher(Publisher::_nil()),
  m_subscriber(Subscriber::_nil())
#endif /* DDS_PROVIDER */
{
	if (m_log != NULL)
	{
		m_participantName = m_log->appName();
	}
}


BasicDdsParticipant::BasicDdsParticipant(DDS::DomainId_t domainId, std::string participantName, AppLog *log)
: m_participantName(participantName),
  m_log(log),
  m_peerMonitoring(NULL),
  m_domainId(domainId),
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
  m_appParticipant(NULL),
  m_publisher(NULL),
  m_subscriber(NULL)
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
  m_appParticipant(DomainParticipant::_nil()),
  m_publisher(Publisher::_nil()),
  m_subscriber(Subscriber::_nil())
#endif /* DDS_PROVIDER */
{

}



BasicDdsParticipant::BasicDdsParticipant(DDS::DomainId_t domainId, string participantName, vector< string > const& privateArgv, AppLog *log)
: m_participantName(participantName),
  m_log(log),
  m_peerMonitoring(NULL),
  m_domainId(domainId),
  m_privateArgv(privateArgv),
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
  m_appParticipant(NULL),
  m_publisher(NULL),
  m_subscriber(NULL)
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
  m_appParticipant(DomainParticipant::_nil()),
  m_publisher(Publisher::_nil()),
  m_subscriber(Subscriber::_nil())
#endif /* DDS_PROVIDER */
{

}


BasicDdsParticipant::~BasicDdsParticipant()
{
	this->stopDds();
	destroy(m_peerMonitoring);

#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
	m_appParticipant = NULL;
	m_publisher = NULL;
	m_subscriber = NULL;
#endif /* DDS_PROVIDER */
}





void BasicDdsParticipant::addTopic(BaseDdsTopic* aTopic)
{
	m_topics.push_back(aTopic);
}


void BasicDdsParticipant::startDds()
{
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
	DomainParticipantFactory *dpf = NULL;
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
	DomainParticipantFactory_var dpf = DomainParticipantFactory::_nil();
#endif /* DDS_PROVIDER */
	DomainParticipantQos dpQos;

#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX) || (DDS_PROVIDER == DDS_PROVIDER_OSPL)
	dpf = DomainParticipantFactory::get_instance();
#elif defined(DDS_PROVIDER) && (DDS_PROVIDER == DDS_PROVIDER_ODDS)
	int argc = m_privateArgv.size() + 1;
	char **argv = (char **)calloc(argc + 1, sizeof(char *));
	argv[0] = strdup("dds_application");
	argv[argc] = NULL; // Terminating sentinel
	for (int idx = 1; idx < argc; idx++)
	{
		argv[idx] = strdup(m_privateArgv[idx - 1].c_str());
	}
	dpf = TheParticipantFactoryWithArgs(argc, argv);
	for (int idx = 0; idx < argc; idx++)
	{
		free(argv[idx]);
	}
	free(argv);
	argv = NULL;
#endif /* DDS_PROVIDER */

#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
	dpf->get_default_participant_qos(&dpQos);
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
	dpf->get_default_participant_qos(dpQos);
#endif /* DDS_PROVIDER */
	this->configureParticipantQos(dpQos);

	if (m_appParticipant || m_publisher || m_subscriber)
	{
		throw PreconditionNotMetException("Application Participant Not Previously Configured");
	}

	m_peerMonitoring = construct(DdsKit::PeerMonitoring::myType(), m_log);
	m_appParticipant = dpf->create_participant(
		m_domainId,
		dpQos,
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
		m_peerMonitoring,
		DDS::SUBSCRIPTION_MATCHED_STATUS |
		DDS::PUBLICATION_MATCHED_STATUS |
		DDS::LIVELINESS_CHANGED_STATUS |
		DDS::LIVELINESS_LOST_STATUS
#else
        // Unfortunately have not been able to confirm that the listeners used
        // to implement the peer monitoring functionality work at all in either
        // OpenSplice or OpenDDS
        nullptr,
        DDS::STATUS_MASK_NONE
#endif /* !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX) */
    );
	if (!m_appParticipant)
	{
		throw DdsErrorException("create_participant", DDS::RETCODE_ERROR);
	}

	m_publisher = m_appParticipant->create_publisher(PUBLISHER_QOS_DEFAULT, NULL, 0);
	if (!m_publisher)
	{
		throw DdsErrorException("create_publisher", DDS::RETCODE_ERROR);
	}

	m_subscriber = m_appParticipant->create_subscriber(SUBSCRIBER_QOS_DEFAULT, NULL, 0);
	if (!m_subscriber)
	{
		throw DdsErrorException("create_subscriber", DDS::RETCODE_ERROR);
	}

	if (m_log != NULL)
	{
		(*m_log) << AppLog::LL_INFO << "Started DDS Participant \"" << m_participantName << "\" on domain (" << m_domainId << ")." << EndLog;
	}
}


void BasicDdsParticipant::stopDds()
{
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
	DomainParticipantFactory *dpf = NULL;
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
	DomainParticipantFactory_var dpf = DomainParticipantFactory::_nil();
#endif /* DDS_PROVIDER */

#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
	dpf = DomainParticipantFactory::get_instance();
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
	dpf = TheParticipantFactory;
#endif /* DDS_PROVIDER */

	if (m_appParticipant)
	{
		for_each(m_topics.begin(), m_topics.end(), ptr_fun(&destroy<BaseDdsTopic>));
		m_topics.clear();
		m_appParticipant->delete_contained_entities();
		dpf->delete_participant(m_appParticipant);
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
		m_publisher = NULL;
		m_subscriber = NULL;
		m_appParticipant = NULL;
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
		m_publisher = DDS::Publisher::_nil();
		m_subscriber = DDS::Subscriber::_nil();
		m_appParticipant = DDS::DomainParticipant::_nil();
#endif /* DDS_PROVIDER */
#if defined(DDS_PROVIDER) && (DDS_PROVIDER == DDS_PROVIDER_ODDS)
		TheServiceParticipant->shutdown();
#endif /* DDS_PROVIDER */

		if (m_log != NULL)
		{
			(*m_log) << AppLog::LL_INFO << "Stopped DDS Participant \"" << m_participantName << "\" on domain (" << m_domainId << ")." << EndLog;
		}
	}
}


#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
void BasicDdsParticipant::preConfigurePeer(string peerIpAddr)
{
	in_addr_t anAddr = INADDR_NONE;

	anAddr = inet_addr(peerIpAddr.c_str());
	if (INADDR_NONE == anAddr)
	{
		throw InvalidInputException("Peer IP Address", peerIpAddr);
	}
	m_peerVector.push_back(anAddr);

}
#endif /* DDS_PROVIDER */


void BasicDdsParticipant::configureParticipantQos(DomainParticipantQos &dpQos)
{
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
	ParticipantLocator pl;
	vector<in_addr_t>::iterator aPeer;

	if (!m_peerVector.empty())
	{
		memset(&pl, 0x00, sizeof(pl));
		for (aPeer = m_peerVector.begin(); aPeer != m_peerVector.end(); ++aPeer)
		{
			pl.participant_locator.kind = COREDX_UDPV4_LOCATOR_KIND_QOS;
			memset(pl.participant_locator.addr, 0x00, sizeof(pl.participant_locator.addr));
			memcpy(&pl.participant_locator.addr[12], &(*aPeer), sizeof(in_addr_t));
			dpQos.peer_participants.value.push_back(pl);
			if (m_log)
			{
				(*m_log) << AppLog::LL_INFO << "Pre-configuring DDS Peer "
						 << (unsigned)pl.participant_locator.addr[12] << "."
						 << (unsigned)pl.participant_locator.addr[13] << "."
						 << (unsigned)pl.participant_locator.addr[14] << "."
						 << (unsigned)pl.participant_locator.addr[15] << EndLog;
			}
			pl.participant_id++;
		}
	}

	if (m_log != NULL)
	{
		memset(&dpQos.entity_name.value, 0x00, sizeof(dpQos.entity_name.value));
		strncpy(dpQos.entity_name.value, m_participantName.c_str(), sizeof(dpQos.entity_name.value) - 1u);
	}
#endif /* DDS_PROVIDER */
}

} // namespace DdsKit
