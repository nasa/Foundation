/**
 * \file BasicDdsParticipant.h
 * \brief Definition of the BasicDdsParticipant Class
 * \date 2012-09-26 17:11:17
 * \author Rolando J. Nieves
 */
#if !defined(EA_C8147D96_6425_4af5_8B1C_3CBB28132307__INCLUDED_)
#define EA_C8147D96_6425_4af5_8B1C_3CBB28132307__INCLUDED_

#include <vector>

#include <arpa/inet.h>

#include <CoreKit/AppLog.h>
#include <CoreKit/Application.h>
#include <CoreKit/factory.h>

#include <DdsKit/dds_provider.h>

#include <DdsKit/BaseDdsTopic.h>
#include <DdsKit/PeerMonitoring.h>

namespace DdsKit
{
	extern const DDS::DomainId_t INVALID_DOMAIN;

        /**
         * \brief Test if a domain number is valid for use
         * \return true if domain is valid, false if invalid
         */
	inline bool IsDomainIdValid(DDS::DomainId_t domainId)
	{ return (domainId >= 0) && (domainId < INVALID_DOMAIN); }

	/**
	 * \brief DDS Participant that Includes Basic Functionality
	 * 
	 * The \c BasicDdsParticipant class models a DDS participant assistant
	 * class that manages the most basic of tasks when interacting in a DDS
	 * domain. \par
	 *
	 * The \c BasicDdsParticipant class assumes that the DDS application using
	 * it expects to use only one \c DDS::Publisher and \c DDS::Subscriber
	 * instance with it. These entities are created automatically when the DDS
	 * functionality of this class is activated. \par
	 *
	 * This class also manages any \c BaseDdsTopic (or \c ConfiguredDdsTopic )
	 * instances that wish to inteact with the DDS domain participant managed
	 * by a particular \c BasicDdsParticipant instance.
	 *
	 * \author Rolando J. Nieves
	 * \date 26-Sep-2012 3:27:03 PM
	 */
	class BasicDdsParticipant
	{
		RF_CK_FACTORY_COMPATIBLE(BasicDdsParticipant);
	public:

                /** \brief Command line flag for specifying the DDS provider */
		static const std::string DDS_PROVIDER_ARG_FLAG;

                /** 
                 * \brief Adds the DDS_PROVIDER_ARG_FLAG to the app command line parameters
                 * \param[out] theApp the application to update
                 */
		static void AddDdsRelatedCmdArgs(CoreKit::Application *theApp);

                /**
                 * \brief Read the DDS specific command line argument values into a vector
                 * \param[in] theApp application instance with command line arguments
                 * \return vector of DDS specific arguments extracted from command line
                 */
		static std::vector< std::string > GatherDdsRelatedCmdArgs(CoreKit::Application *theApp);
		
		/**
		 * \brief Initialize Instance Fields to Nominal Values
		 *
		 * The primary constructor of the \c BasicDdsParticipant class
		 * initializes all instance fields, including the field that stores the
		 * DDS domain ID that the \c DDS::DomainParticipant instance managed by
		 * this instance will participate on.
		 *
		 * \param domainId DDS domain ID of the domain this participant will
		 *                 interact on.
                 * \param log the application log instance
		 */
		BasicDdsParticipant(DDS::DomainId_t domainId, CoreKit::AppLog *log = NULL);

		/**
		 * \brief Constructor
		 *
		 * \param domainId DDS domain ID of the domain this participant will
		 *                 interact on.
                 * \param participantName the name of the DDS participant
                 * \param log the application log instance
		 */
		BasicDdsParticipant(DDS::DomainId_t domainId, std::string participantName, CoreKit::AppLog *log = NULL);

		/**
		 * \brief Constructor
		 *
		 * \param domainId DDS domain ID of the domain this participant will
		 *                 interact on.
                 * \param participantName the name of the DDS participant
                 * \param privateArgv  OpenDDS specific arguments
                 * \param log the application log instance
		 */
		BasicDdsParticipant(DDS::DomainId_t domainId, std::string participantName, std::vector< std::string > const& privateArgv, CoreKit::AppLog *log);

		/**
		 * \brief Deactivate All DDS Services Owned by This Instance
		 *
		 * The destructor for the \c BasicDdsParticipant ensures that all DDS
		 * entities and services owned by this instance are deactivated and
		 * destroyed.
		 */
		virtual ~BasicDdsParticipant();

		/**
		 * \brief Access the \c DDS::DomainParticipant Instance Managed by This Instance
		 *
		 * \return \c DDS::DomainParticipant instance created and managed by
		 *         this instance.
		 */
		inline
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
			DDS::DomainParticipant *const appParticipant() const { return m_appParticipant; }
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
			DDS::DomainParticipant_ptr appParticipant() const { return m_appParticipant.in(); }
#endif /* DDS_PROVIDER */

		/**
		 * \brief Access the \c DDS::Publisher Instance Managed by This Instance
		 *
		 * \return \c DDS::Publisher instance created and managed by this
		 *         instance.
		 */
		inline
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
			DDS::Publisher *const publisher() const { return m_publisher; }
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
			DDS::Publisher_ptr publisher() const { return m_publisher.in(); }
#endif /* DDS_PROVIDER */
			
		/**
		 * \brief Access the \c DDS::Subscriber Instance Managed by This Instance
		 *
		 * \return \c DDS::Subscriber instance created and managed by this
		 *         instance.
		 */
		inline
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
			DDS::Subscriber *const subscriber() const { return m_subscriber; }
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
			DDS::Subscriber_ptr subscriber() const { return m_subscriber.in(); }
#endif /* DDS_PROVIDER */
			

                /**
                 * \brief Get the peer monitoring instance
                 * \return the peer monitoring instance
                 */
		inline PeerMonitoring *const usePeerMonitoring() { return m_peerMonitoring; }

                /**
                 * \brief  Get the application log
                 * \return the application log
                 */
		inline CoreKit::AppLog *const useLog() { return m_log; }

		/**
		 * \brief Add and \b Adopt a DDS Topic to This Participant
		 *
		 * The \c addTopic() method is used to register a new DDS topic for use
		 * by the DDS participant managed in this \c BasicDdsParticipant
		 * instance. The instance submitted via this method is \b adopted by
		 * this object, meaning the topic instance will be destroyed when this
		 * instance deactivates all DDS services (via \c stopDds() ).
		 *
		 * \note
		 * Before adding topics with this method, the application using this
		 * \c BasicDdsParticipant instance must call the \c startDds() method.
		 *
		 * \param aTopic \c BaseDdsTopic instance that manages the DDS topic
		 *               being added to this participant.
		 */
		void addTopic(BaseDdsTopic* aTopic);
		/**
		 * \brief Start the DDS Subsystem.
		 *
		 * The \c startDds() method is used to create all the necessary
		 * entities to begin DDS communications. When used in conjunction
		 * with a \c CoreKit::AppDelegate derived class, it is best to call
		 * this method during the
		 * \c CoreKit::AppDelegate::applicationStarting() method.
		 */
		virtual void startDds();
		/**
		 * \brief Stop the DDS Subsystem.
		 *
		 * The \c stopDds() method is used to deactivate and destroy all
		 * entities that supported DDS communications. When used in
		 * conjunction with a \c CoreKit::AppDelegate derived class, it is best
		 * to call this method during the
		 * \c CoreKit::AppDelegate::applicationDidTerminate() method.
		 */
		virtual void stopDds();

#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
		virtual void preConfigurePeer(std::string peerIpAddr);
#endif /* DDS_PROVIDER == DDS_PROVIDER_COREDX */

	protected:
		/**
		 * \brief \c DDS::DomainParticipant Instance Used for DDS Communications
		 */
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
		DDS::DomainParticipant*
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
		DDS::DomainParticipant_var
#endif /* DDS_PROVIDER */
			m_appParticipant;
		/**
		 * \brief \c DDS::Publisher Instance Used for DDS Communications
		 */
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
		DDS::Publisher*
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
		DDS::Publisher_var
#endif /* DDS_PROVIDER */
			m_publisher;
		/**
		 * \brief \c DDS::Subscriber Instance Used for DDS Communications
		 */
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
		DDS::Subscriber*
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
		DDS::Subscriber_var
#endif /* DDS_PROVIDER */
			m_subscriber;
		/**
		 * \brief \c Collection of DDS Topics That Rely on This Object for DDS Infrastructure
		 */
		std::vector<BaseDdsTopic*> m_topics;
		/**
		 * \brief ID for the DDS Domain This Object will Participate In
		 */
		DDS::DomainId_t m_domainId;

#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
		std::vector<in_addr_t> m_peerVector;
#endif /* DDS_PROVIDER == DDS_PROVIDER_COREDX */

		CoreKit::AppLog *m_log;

		std::string m_participantName;

		PeerMonitoring *m_peerMonitoring;

		std::vector< std::string > m_privateArgv;
		
		/**
		 * \brief Perform Participant QoS Configuration Prior to Creation
		 *
		 * The \c configureParticipantQos() method exists for the benefit of
		 * derived classes that wish to include a set of Quality-of-Service
		 * policies prior to creating a domain participant. The default
		 * implementation of this method is blank.
		 *
		 * \note
		 * Some \c DDS::DomainParticipant QoS policies may be altered after
		 * creation, but some are immutable after participant instantiation.
		 * Hence the need for a mechanism like this.
		 *
		 * \param dpQos Reference to the Domain Participant QoS object that
		 *              will hold settings to apply at participant creation
		 *              time.
		 */
		virtual void configureParticipantQos(DDS::DomainParticipantQos &dpQos);
	};

}
#endif // !defined(EA_C8147D96_6425_4af5_8B1C_3CBB28132307__INCLUDED_)
