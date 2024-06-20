/**
 * \file PeerMonitoring.h
 * \brief Contains the definition of the \c DdsKit::PeerMonitoring class.
 * \date 2013-09-13 17:31:00
 * \author Rolando J. Nieves
 */

#ifndef PEERMONITORING_H_
#define PEERMONITORING_H_

#include <map>
#include <string>
#include <set>
#include <functional>
#include <vector>

#include <CoreKit/factory.h>
#include <CoreKit/InputSource.h>
#include <CoreKit/AppLog.h>

#include <DdsKit/dds_provider.h>

namespace DdsKit {

/**
 * \brief Allows the participant to monitor peers who publish and subscribe on DDS topics
 * \author Rolando J. Nieves
 * \date 2013-09-13
 */
class PeerMonitoring : public DDS::DomainParticipantListener, public CoreKit::InputSource, public CoreKit::InterruptListener
{
	RF_CK_FACTORY_COMPATIBLE(PeerMonitoring);

	class CallbackBase : public std::unary_function<PeerMonitoring*, void>
	{
	public:
		virtual ~CallbackBase()
		{}
		virtual void operator()(PeerMonitoring *theMonitor) = 0;
	};

public:
	typedef std::set<std::string> PubSubNameMap_t;

    /**
     * \brief Wrapper for callback for peer monitioring notificaitons
     * \tparam CallbackTarget the callback type
     * \author Rolando J. Nieves
     * \date 2013-09-13
     */
	template<typename CallbackTarget>
	class Callback : public CallbackBase
	{
		RF_CK_FACTORY_COMPATIBLE(PeerMonitoring::Callback<CallbackTarget>);
	public:
		explicit Callback(CallbackTarget& cbTarget)
		: m_cbTarget(cbTarget)
		{}
		virtual ~Callback()
		{}
		virtual void operator()(PeerMonitoring *theMonitor)
		{
			m_cbTarget(theMonitor);
		}
	private:
		CallbackTarget m_cbTarget;
	};

    /**
     * \brief Create a newly allocated callback wrapper for the target
     * \tparam CallbackTarget the callback type
     * \param cbTarget the callback target.
     * \return newly allocated callback
     */
	template<typename CallbackTarget>
	static CallbackBase* newCallbackWithTarget(CallbackTarget cbTarget)
	{ return CoreKit::construct(Callback<CallbackTarget>::myType(), cbTarget); }

    /**
     * \brief Contructor
     * \param appLog the application log instance
     */
	explicit PeerMonitoring(CoreKit::AppLog *appLog = nullptr);

    /**
     * \brief Destructor
     */
	virtual ~PeerMonitoring();

    /**
     * \brief Get the matched publications
     * \return matched publications
     */
	inline PubSubNameMap_t const& matchedPublications() const { return m_matchedPublications; }

    /**
     * \brief Get the matched subscriptions
     * \return matched subscriptions
     */
	inline PubSubNameMap_t const& matchedSubscriptions() const { return m_matchedSubscriptions; }

    /**
     * \brief Initialize to operate with the \c CoreKit::RunLoop
     */
	void initializeRunLoopOperation();

    /**
     * \brief Register callback for notifications of publication peer events
     * \param pubChangeCb the callback
     */
	void registerPublicationChangeCallback(CallbackBase *pubChangeCb);

    /**
     * \brief Register callback for notifications of subscription peer events
     * \param subChangeCb the callback
     */
	void registerSubscriptionChangeCallback(CallbackBase *subChangeCb);

	virtual int fileDescriptor() const
	{ return m_inputPipe[0]; }

	virtual CoreKit::InterruptListener* interruptListener() const
	{ return const_cast<PeerMonitoring *>(this); }

	virtual void inputAvailableFrom(CoreKit::InputSource *theSource);

	virtual void fireCallback() { this->inputAvailableFrom(this); }

#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
	virtual void on_publication_matched(DDS::DataWriter *theWriter, DDS::PublicationMatchedStatus status);
	virtual void on_liveliness_lost(DDS::DataWriter *theWriter, DDS::LivelinessLostStatus status);
	virtual void on_subscription_matched(DDS::DataReader *theReader, DDS::SubscriptionMatchedStatus status);
	virtual void on_liveliness_changed(DDS::DataReader *theReader, DDS::LivelinessChangedStatus status);
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
	virtual void on_publication_matched(DDS::DataWriter_ptr theWriter, DDS::PublicationMatchedStatus const& status)
# if (DDS_PROVIDER == DDS_PROVIDER_ODDS)
		throw (CORBA::SystemException)
# endif /* DDS_PROVIDER == DDS_PROVIDER_ODDS */
	;
	virtual void on_liveliness_lost(DDS::DataWriter_ptr theWriter, DDS::LivelinessLostStatus const& status)
# if (DDS_PROVIDER == DDS_PROVIDER_ODDS)
		throw (CORBA::SystemException)
# endif /* DDS_PROVIDER == DDS_PROVIDER_ODDS */
	;
	virtual void on_subscription_matched(DDS::DataReader_ptr theReader, DDS::SubscriptionMatchedStatus const& status)
# if (DDS_PROVIDER == DDS_PROVIDER_ODDS)
		throw (CORBA::SystemException)
# endif /* DDS_PROVIDER == DDS_PROVIDER_ODDS */
	;
	virtual void on_liveliness_changed(DDS::DataReader_ptr theReader, DDS::LivelinessChangedStatus const& status)
# if (DDS_PROVIDER == DDS_PROVIDER_ODDS)
		throw (CORBA::SystemException)
# endif /* DDS_PROVIDER == DDS_PROVIDER_ODDS */
	;
	virtual void on_inconsistent_topic(DDS::Topic_ptr theTopic, DDS::InconsistentTopicStatus const& status)
# if (DDS_PROVIDER == DDS_PROVIDER_ODDS)
		throw (CORBA::SystemException)
# endif /* DDS_PROVIDER == DDS_PROVIDER_ODDS */
	{}
	virtual void on_data_on_readers(DDS::Subscriber_ptr subs)
# if (DDS_PROVIDER == DDS_PROVIDER_ODDS)
		throw (CORBA::SystemException)
# endif /* DDS_PROVIDER == DDS_PROVIDER_ODDS */
	{}
	virtual void on_sample_lost(DDS::DataReader_ptr reader, DDS::SampleLostStatus const& status)
# if (DDS_PROVIDER == DDS_PROVIDER_ODDS)
		throw (CORBA::SystemException)
# endif /* DDS_PROVIDER == DDS_PROVIDER_ODDS */
	{}
	virtual void on_data_available(DDS::DataReader_ptr reader)
# if (DDS_PROVIDER == DDS_PROVIDER_ODDS)
		throw (CORBA::SystemException)
# endif /* DDS_PROVIDER == DDS_PROVIDER_ODDS */
	{}
	virtual void on_sample_rejected(DDS::DataReader_ptr reader, DDS::SampleRejectedStatus const& status)
# if (DDS_PROVIDER == DDS_PROVIDER_ODDS)
		throw (CORBA::SystemException)
# endif /* DDS_PROVIDER == DDS_PROVIDER_ODDS */
	{}
	virtual void on_requested_incompatible_qos(DDS::DataReader_ptr reader, DDS::RequestedIncompatibleQosStatus const& status)
# if (DDS_PROVIDER == DDS_PROVIDER_ODDS)
		throw (CORBA::SystemException)
# endif /* DDS_PROVIDER == DDS_PROVIDER_ODDS */
	{}
	virtual void on_requested_deadline_missed(DDS::DataReader_ptr reader, DDS::RequestedDeadlineMissedStatus const& status)
# if (DDS_PROVIDER == DDS_PROVIDER_ODDS)
		throw (CORBA::SystemException)
# endif /* DDS_PROVIDER == DDS_PROVIDER_ODDS */
	{}
	virtual void on_offered_incompatible_qos(DDS::DataWriter_ptr reader, DDS::OfferedIncompatibleQosStatus const& status)
# if (DDS_PROVIDER == DDS_PROVIDER_ODDS)
		throw (CORBA::SystemException)
# endif /* DDS_PROVIDER == DDS_PROVIDER_ODDS */
	{}
	virtual void on_offered_deadline_missed(DDS::DataWriter_ptr writer, DDS::OfferedDeadlineMissedStatus const& status)
# if (DDS_PROVIDER == DDS_PROVIDER_ODDS)
		throw (CORBA::SystemException)
# endif /* DDS_PROVIDER == DDS_PROVIDER_ODDS */
	{}
#endif /* DDS_PROVIDER */

private:
	typedef std::map<std::string, std::string> KeyToNameMap_t;
	typedef std::vector<CallbackBase*> ChangeCallbackList_t;

	KeyToNameMap_t m_participantNameMap;
	PubSubNameMap_t m_matchedPublications;
	PubSubNameMap_t m_matchedSubscriptions;
	ChangeCallbackList_t m_pubChangeCbList;
	ChangeCallbackList_t m_subChangeCbList;
	int m_inputPipe[2];
	CoreKit::AppLog *m_appLog;

	bool refreshParticipantNameMap(
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
		DDS::DomainParticipant*
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
		DDS::DomainParticipant_ptr
#endif /* DDS_PROVIDER */
		myParticip
		);
	void refreshSubscriptionsOnWriter(
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
		DDS::DataWriter*
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
		DDS::DataWriter_ptr
#endif /* DDS_PROVIDER */
		theWriter
		);
	void refreshPublicationsOnReader(
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
		DDS::DataReader*
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
		DDS::DataReader_ptr
#endif /* DDS_PROVIDER */
		theReader);
	void dispatchPublicationChangeCallback();
	void dispatchSubscriptionChangeCallback();
	void indicateChange(uint8_t whatChanged);
};

/**
 * \brief Generate a text summary for a built-in topic.
 *
 * The built-in topic is identified by its key.
 *
 * \param theKey Key that identifies the built-in topic.
 *
 * \return Text describing the built-in topic.
 */
std::string BuiltinTopicKeyToString(DDS::BuiltinTopicKey_t const& theKey);

/**
 * \brief Generate a participant name.
 *
 * The participant is identified via its unique key.
 *
 * \param theKey Key that identifies the participant.
 *
 * \return Text describing the participant.
 */
std::string CreateParticipNameFromKey(DDS::BuiltinTopicKey_t const& theKey);

} /* namespace DdsKit */

#endif /* PEERMONITORING_H_ */

// vim: set ts=4 sw=4 expandtab:
