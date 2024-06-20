/**
 * \file PeerMonitoring.cpp
 * \brief Contains the definition of the \c DdsKit::PeerMonitoring class.
 * \date 2013-09-13 17:47:00
 * \author Rolando J. Nieves
 */

#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <algorithm>
#include <sstream>

#include <CoreKit/OsErrorException.h>

#include <DdsKit/dds_provider.h>

#include "PeerMonitoring.h"

#define DK_PM_SUBS_CHANGED ('R')
#define DK_PM_PUBS_CHANGED ('W')

using std::string;
using std::stringstream;
using std::for_each;
using std::bind2nd;
using std::ptr_fun;
using std::mem_fun;
using CoreKit::destroy;
using CoreKit::InputSource;
using CoreKit::OsErrorException;
using CoreKit::AppLog;
using CoreKit::EndLog;
using CoreKit::format;
using DDS::InstanceHandleSeq;
using DDS::DataWriter;
using DDS::PublicationMatchedStatus;
using DDS::LivelinessLostStatus;
using DDS::DataReader;
using DDS::SubscriptionMatchedStatus;
using DDS::LivelinessChangedStatus;
using DDS::DomainParticipant;
#if defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
using DDS::DomainParticipant_ptr;
using DDS::DataReader_ptr;
using DDS::DataWriter_ptr;
# if (DDS_PROVIDER == DDS_PROVIDER_ODDS)
using CORBA::SystemException;
# endif /* DDS_PROVIDER == DDS_PROVIDER_ODDS */
#endif /* DDS_PROVIDER */
using DDS::ReturnCode_t;
using DDS::RETCODE_OK;
using DDS::ParticipantBuiltinTopicData;
using DDS::InstanceHandle_t;
using DDS::BuiltinTopicKey_t;
using DDS::SubscriptionBuiltinTopicData;
using DDS::PublicationBuiltinTopicData;

namespace DdsKit {

PeerMonitoring::PeerMonitoring(AppLog *appLog)
: m_appLog(appLog)
{
    m_inputPipe[0] = -1;
    m_inputPipe[1] = -1;
}


PeerMonitoring::~PeerMonitoring()
{
    for_each(m_pubChangeCbList.begin(), m_pubChangeCbList.end(), ptr_fun(&CoreKit::destroy<CallbackBase>));
    m_pubChangeCbList.clear();
    for_each(m_subChangeCbList.begin(), m_subChangeCbList.end(), ptr_fun(&CoreKit::destroy<CallbackBase>));
    m_subChangeCbList.clear();

    if (m_inputPipe[0] != -1)
    {
        close(m_inputPipe[0]);
        m_inputPipe[0] = -1;
    }

    if (m_inputPipe[1] != -1)
    {
        close(m_inputPipe[1]);
        m_inputPipe[1] = -1;
    }
}




void PeerMonitoring::initializeRunLoopOperation()
{
#if defined(HAVE_PIPE2) && (HAVE_PIPE2 == 1)
    if (pipe2(m_inputPipe, O_NONBLOCK | O_CLOEXEC) != 0)
#else
    if (pipe(m_inputPipe) != 0)
#endif /* defined(HAVE_PIPE2) && (HAVE_PIPE2 == 1)*/
    {
        throw OsErrorException("pipe2", errno);
    }
#if !defined(HAVE_PIPE2) || (HAVE_PIPE2 != 1)
    fcntl(m_inputPipe[0], F_SETFD, O_CLOEXEC);
    fcntl(m_inputPipe[0], F_SETFL, O_NONBLOCK);
#endif /* !defined(HAVE_PIPE2) || (HAVE_PIPE2 != 1) */
}


void PeerMonitoring::registerPublicationChangeCallback(PeerMonitoring::CallbackBase *pubChangeCb)
{
    if (pubChangeCb != nullptr)
    {
        m_pubChangeCbList.push_back(pubChangeCb);
    }
}


void PeerMonitoring::registerSubscriptionChangeCallback(PeerMonitoring::CallbackBase *subChangeCb)
{
    if (subChangeCb != nullptr)
    {
        m_subChangeCbList.push_back(subChangeCb);
    }
}


void PeerMonitoring::inputAvailableFrom(InputSource *theSource)
{
    uint8_t theChange;

    if (-1 == m_inputPipe[0])
    {
        return;
    }

    if (read(m_inputPipe[0], &theChange, 1) != 1)
    {
        if (m_appLog != nullptr)
        {
            (*m_appLog) << AppLog::LL_WARNING << "Unable to accept input stimulus from DDS thread: "
                        << strerror(errno) << EndLog;
        }
        return;
    }

    switch(theChange)
    {
    case DK_PM_PUBS_CHANGED:
        this->dispatchPublicationChangeCallback();
        break;
    case DK_PM_SUBS_CHANGED:
        this->dispatchSubscriptionChangeCallback();
        break;
    default:
        if (m_appLog != nullptr)
        {
            (*m_appLog) << AppLog::LL_WARNING << "Unknown stimulus received from DDS thread: "
                        << format("%02X", theChange) << "(" << (char)theChange << ")." << EndLog;
        }
        break;
    }
}


void
PeerMonitoring::on_publication_matched(
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
    DataWriter *theWriter,
    PublicationMatchedStatus status
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
    DataWriter_ptr theWriter,
    PublicationMatchedStatus const& status
#endif /* DDS_PROVIDER */
    )
#if defined(DDS_PROVIDER) && (DDS_PROVIDER == DDS_PROVIDER_ODDS)
    throw (SystemException)
#endif /* DDS_PROVIDER == DDS_PROVIDER_ODDS */
{
    this->refreshSubscriptionsOnWriter(theWriter);
    this->indicateChange(DK_PM_PUBS_CHANGED);
}


void
PeerMonitoring::on_liveliness_lost(
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
    DataWriter *theWriter,
    LivelinessLostStatus status
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
    DataWriter_ptr theWriter,
    LivelinessLostStatus const& status
#endif /* DDS_PROVIDER */
    )
#if defined(DDS_PROVIDER) && (DDS_PROVIDER == DDS_PROVIDER_ODDS)
    throw (SystemException)
#endif /* DDS_PROVIDER == DDS_PROVIDER_ODDS */
{
    this->refreshSubscriptionsOnWriter(theWriter);
    this->indicateChange(DK_PM_PUBS_CHANGED);
}


void
PeerMonitoring::on_subscription_matched(
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
    DataReader *theReader,
    SubscriptionMatchedStatus status
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
    DataReader_ptr theReader,
    SubscriptionMatchedStatus const& status
#endif /* DDS_PROVIDER */
    )
#if defined(DDS_PROVIDER) && (DDS_PROVIDER == DDS_PROVIDER_ODDS)
    throw (SystemException)
#endif /* DDS_PROVIDER == DDS_PROVIDER_ODDS */
{
    this->refreshPublicationsOnReader(theReader);
    this->indicateChange(DK_PM_SUBS_CHANGED);
}


void
PeerMonitoring::on_liveliness_changed(
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
    DataReader *theReader,
    LivelinessChangedStatus status
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
    DataReader_ptr theReader,
    LivelinessChangedStatus const& status
#endif /* DDS_PROVIDER */
    )
#if defined(DDS_PROVIDER) && (DDS_PROVIDER == DDS_PROVIDER_ODDS)
    throw (SystemException)
#endif /* DDS_PROVIDER == DDS_PROVIDER_ODDS */
{
    this->refreshPublicationsOnReader(theReader);
    this->indicateChange(DK_PM_SUBS_CHANGED);
}


bool
PeerMonitoring::refreshParticipantNameMap(
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
    DomainParticipant*
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
    DomainParticipant_ptr
#endif /* DDS_PROVIDER */
    myParticip)
{
    InstanceHandleSeq discoveredParticips;
    ReturnCode_t ddsRetCode = RETCODE_OK;
    ParticipantBuiltinTopicData aParticipData;
    InstanceHandle_t handle;
    string participName;

    ddsRetCode = myParticip->get_discovered_participants(
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
        &discoveredParticips
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
        discoveredParticips
#endif /* DDS_PROVIDER */
        );
    if (ddsRetCode != RETCODE_OK)
    {
        if (m_appLog != nullptr)
        {
            (*m_appLog) << AppLog::LL_WARNING
                        << "Unable to acquire list of discovered participants: "
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
                        << DDS_error(ddsRetCode)
#endif /* DDS_PROVIDER */
                        << EndLog;
        }
        return false;
    }
    m_participantNameMap.clear();
    for (uint32_t idx = 0u;
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
         idx < discoveredParticips.size();
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
         idx < discoveredParticips.length();
#endif /* DDS_PROVIDER */
         idx++)
    {
        handle = discoveredParticips[idx];
        ddsRetCode = myParticip->get_discovered_participant_data(
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
            &aParticipData,
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
            aParticipData,
#endif /* DDS_PROVIDER */
            handle);
        if (ddsRetCode != RETCODE_OK)
        {
            if (m_appLog != nullptr)
            {
                (*m_appLog) << AppLog::LL_WARNING << "Unable to obtain information on a discovered participant with handle ("
                            << format("%08X", (uint32_t)handle) << ")" << EndLog;
            }
        
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
            aParticipData.user_data.clear();
            aParticipData.clear();
#endif /* DDS_PROVIDER */
            continue;
        }
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
        // Is the discovered peer a fellow CoreDX user?
        if (aParticipData.entity_name != nullptr)
        {
            // Yes? then use the "entity_name"
            participName = aParticipData.entity_name;
        }
        else
        {
            // No? then generic name it is
            participName = CreateParticipNameFromKey(aParticipData.key);
        }
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
        // Generic names for both OpenSplice and OpenDDS
        participName = CreateParticipNameFromKey(aParticipData.key);
#endif /* DDS_PROVIDER */
        m_participantNameMap[BuiltinTopicKeyToString(aParticipData.key)] = participName;
        
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
        aParticipData.user_data.clear();
        aParticipData.clear();
#endif /* DDS_PROVIDER */
    }

    return true;
}


void
PeerMonitoring::refreshSubscriptionsOnWriter(
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
    DataWriter*
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
    DataWriter_ptr
#endif /* DDS_PROVIDER */
    theWriter)
{
    InstanceHandleSeq matchedSubs;
    InstanceHandle_t aSubHandle;
    SubscriptionBuiltinTopicData aSubInfo;
    string subLabel;
    ReturnCode_t ddsRetCode = RETCODE_OK;
    string participantName = "<unknown>";
    string topicName = "<unknown>";
    KeyToNameMap_t::const_iterator participNameEntry;

    if (!this->refreshParticipantNameMap(theWriter->get_publisher()->get_participant()))
    {
        return;
    }

    ddsRetCode = theWriter->get_matched_subscriptions(
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
        &matchedSubs
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
        matchedSubs
#endif /* DDS_PROVIDER */
        );
    if (ddsRetCode != RETCODE_OK)
    {
        if (m_appLog != nullptr)
        {
            if (theWriter->get_topic())
            {
                topicName = theWriter->get_topic()->get_name();
            }
            (*m_appLog) << AppLog::LL_WARNING << "Unable to obtain list of matched subscriptions on writer for topic \""
                        << topicName << "\"" << EndLog;
        }
        return;
    }
    m_matchedSubscriptions.clear();
    for (uint32_t idx = 0u;
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
         idx < matchedSubs.size();
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
         idx < matchedSubs.length();
#endif /* DDS_PROVIDER */
         idx++)
    {
        aSubHandle = matchedSubs[idx];
        ddsRetCode = theWriter->get_matched_subscription_data(
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
            &aSubInfo,
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
            aSubInfo,
#endif /* DDS_PROVIDER */
            aSubHandle
            );
        if (ddsRetCode != RETCODE_OK)
        {
            if (m_appLog != nullptr)
            {
                (*m_appLog) << AppLog::LL_WARNING << "Unable to obtain subscription information for data reader with handle ("
                            << format("%08X", (uint32_t)aSubHandle) << ")" << EndLog;
            }
            continue;
        }
        participNameEntry = m_participantNameMap.find(BuiltinTopicKeyToString(aSubInfo.participant_key));
        if (participNameEntry != m_participantNameMap.end())
        {
            participantName = (*participNameEntry).second;
        }
        subLabel = participantName + string("::") + (const char*)aSubInfo.topic_name + string("DR");
        m_matchedSubscriptions.insert(subLabel);
    }
}


void PeerMonitoring::refreshPublicationsOnReader(DDS::DataReader *theReader)
{
    InstanceHandleSeq matchedPubs;
    InstanceHandle_t aPubHandle;
    PublicationBuiltinTopicData aPubInfo;
    stringstream pubLabel;
    ReturnCode_t ddsRetCode = RETCODE_OK;
    string topicName = "<unknown>";

    if (!this->refreshParticipantNameMap(theReader->get_subscriber()->get_participant()))
    {
        return;
    }

    ddsRetCode = theReader->get_matched_publications(
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
        &matchedPubs
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
        matchedPubs
#endif /* DDS_PROVIDER */
        );
    if (ddsRetCode != RETCODE_OK)
    {
        if (m_appLog != nullptr)
        {
            if (theReader->get_topicdescription())
            {
                topicName = theReader->get_topicdescription()->get_name();
            }
            (*m_appLog) << AppLog::LL_WARNING << "Unable to obtain list of matched publications on reader for topic \""
                        << topicName << "\"" << EndLog;
        }
        return;
    }
    m_matchedPublications.clear();
    for (uint32_t idx = 0u;
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
         idx < matchedPubs.size();
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
         idx < matchedPubs.length();
#endif /* DDS_PROVIDER */
         idx++)
    {
        aPubHandle = matchedPubs[idx];
        ddsRetCode = theReader->get_matched_publication_data(
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
            &aPubInfo,
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
            aPubInfo,
#endif /* DDS_PROVIDER */
            aPubHandle);
        if (ddsRetCode != RETCODE_OK)
        {
            if (m_appLog != nullptr)
            {
                (*m_appLog) << AppLog::LL_WARNING << "Unable to obtain publication information for data writer with handle ("
                            << format("%08X", (uint32_t)aPubHandle) << ")" << EndLog;
            }
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
            aPubInfo.clear();
#endif /* DDS_PROVIDER */
            continue;
        }
        pubLabel << m_participantNameMap[BuiltinTopicKeyToString(aPubInfo.participant_key)] << "::" << (const char *)aPubInfo.topic_name << "DW";
        m_matchedPublications.insert(pubLabel.str());
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
        aPubInfo.clear();
#endif /* DDS_PROVIDER */
        pubLabel.str(string());
    }
}


void PeerMonitoring::dispatchPublicationChangeCallback()
{
    for_each(m_pubChangeCbList.begin(), m_pubChangeCbList.end(),
            bind2nd(mem_fun(&CallbackBase::operator()), this));
}


void PeerMonitoring::dispatchSubscriptionChangeCallback()
{
    for_each(m_subChangeCbList.begin(), m_subChangeCbList.end(),
            bind2nd(mem_fun(&CallbackBase::operator()), this));
}


void PeerMonitoring::indicateChange(uint8_t whatChanged)
{
    if (-1 == m_inputPipe[1])
    {
        return;
    }

    if (write(m_inputPipe[1], &whatChanged, 1) != 1)
    {
        if (m_appLog != nullptr)
        {
            (*m_appLog) << AppLog::LL_WARNING << "Unable to send input stimulus from DDS thread: "
                      << strerror(errno) << EndLog;
        }
    }
}

string BuiltinTopicKeyToString(BuiltinTopicKey_t const& theKey)
{
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX) || (DDS_PROVIDER == DDS_PROVIDER_ODDS)
    return CoreKit::format("%08X", theKey.value[0]) +
        CoreKit::format("%08X", theKey.value[1]) +
        CoreKit::format("%08X", theKey.value[2]);
#elif defined(DDS_PROVIDER) && (DDS_PROVIDER == DDS_PROVIDER_OSPL)
    return CoreKit::format("%08X", theKey[0]) +
        CoreKit::format("%08X", theKey[1]) +
        CoreKit::format("%08X", theKey[2]);
#endif /* DDS_PROVIDER */
}

string CreateParticipNameFromKey(BuiltinTopicKey_t const& theKey)
{
    stringstream result;

#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX) || (DDS_PROVIDER == DDS_PROVIDER_ODDS)
    result << CoreKit::format("%08X:", theKey.value[0])
           << CoreKit::format("%08X:", theKey.value[1])
           << CoreKit::format("%08X", theKey.value[2]);
#elif defined(DDS_PROVIDER) && (DDS_PROVIDER == DDS_PROVIDER_OSPL)
    result << CoreKit::format("%08X:", theKey[0])
           << CoreKit::format("%08X:", theKey[1])
           << CoreKit::format("%08X", theKey[2]);
#endif /* DDS_PROVIDER */

    return result.str();
}

} /* namespace DdsKit */
