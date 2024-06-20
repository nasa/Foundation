/**
 * \file ConfiguredDdsTopic.h
 * \brief Definition of the ConfiguredDdsTopic Class
 * \date 2012-09-26 17:52:00
 * \author Rolando J. Nieves
 */
#if !defined(EA_F3240A50_DB91_4c1e_99FE_0AB644DDC500__INCLUDED_)
#define EA_F3240A50_DB91_4c1e_99FE_0AB644DDC500__INCLUDED_

#include <pthread.h>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <vector>
#include <functional>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <limits>

#include <CoreKit/PreconditionNotMetException.h>
#include <CoreKit/OsErrorException.h>
#include <CoreKit/factory.h>
#include <CoreKit/BlockGuard.h>

#include <DdsKit/BaseDdsTopic.h>
#include <DdsKit/BasicDdsParticipant.h>
#include <DdsKit/DdsErrorException.h>
#include <DdsKit/ThroughputCap.h>

namespace DdsKit
{

    /**
     * \brief Concrete Template Class for All DDS Topics Collaborating with \c BasicDdsParticipant.
     *
     * The \c ConfiguredDdsTopic template class contains all the logic required
     * to manage DDS topics as well as transact in DDS samples. The template
     * class uses its template argument to complete its implementation, and
     * tailor itself to a DDS topic. \par
     *
     * The \c ConfiguredDdsTopic template class configures itself for any topic
     * by accepting as its template argument a class or data structure that
     * defines the following data types:
     *
     * - \c SampleType with the C++ class generated for the topic samples
     * - \c SampleSeqType with the C++ class generated for sequences of topic 
     * samples
     * - \c TypeSupportType with the C++ class generated for the topic type
     * support.
     * - \c ReaderPtrType with the C++ class pointer type for the topic data
     * reader. Some DDS provides don't use standard C++ pointers in favor of
     * reference counting (or "smart") pointers, hence the need to explicitly
     * define the pointer type.
     * - \c ReaderType with the C++ class type for the topic data reader.
     * - \c WriterPtrType with the C++ class pointer type for the topic data
     * writer. Some DDS provides don't use standard C++ pointers in favor of
     * reference counting (or "smart") pointers, hence the need to explicitly
     * define the pointer type.
     * - \c WriterType with the C++ class type for the topic data writer.
     * - \c TopicName with a class scope constant (i.e., \c static ) of type
     * \c std::string that contains the character string name to use for the
     * topic in case a specific topic name is not specified.
     * - \c NormallyReliable with a class scope constant of type \c bool that
     * contains \c true if the topic normally uses reliable transport and
     * \c false if it does not.
     * - \c DefaultHistoryDepth with a class scope constant of type \c unsigned
     * that contains the default size of the topic's history depth.
     * - \c TypeNameAlias with a normalized type name for the topic type.
     * It is most useful to normalize the type alias by expressing the topic
     * type fully qualified with its containing modules (e.g., a type "MyType"
     * contained in the "MyModule" module would best use a type alias of
     * "MyModule::MyType", making it compatible with most DDS providers).
     * 
     * The \c ConfiguredDdsTopic template class implements the
     * \c DDS::DataReaderListener interface so that it may bridge the gap
     * between the DDS implementation and \c CoreKit::RunLoop when it comes to
     * asynchronous sample arrival notifications. \par
     *
     * In order to bridge the gap between what may be DDS provider created
     * threads and application managed threads, this template class (when
     * instructed to listen for samples) registers itself as a
     * \c DDS::DataReaderListener . When samples arrive and the
     * \c on_data_available() method is called, this class reads all available
     * samples and copies them to an internal cache. After interacting with the
     * \c DDS::DataReader , this class notifies its associated
     * \c CoreKit::RunLoop and, when scheduled, distributes the received
     * samples from its cache and not from memory and threads managed by the
     * DDS provider.
     *
     * \author Rolando J. Nieves
     * \date 2012-09-26 15:27:40
     */
    template<class TopicConfig>
    class ConfiguredDdsTopic : public BaseDdsTopic, public DDS::DataReaderListener
    {
        RF_CK_FACTORY_COMPATIBLE(ConfiguredDdsTopic<TopicConfig>);
    public:
        /**
         * \brief Template Class Used to Model Callback Objects to Handle Incoming Samples
         *
         * The \c SampleCallback inner class for \c ConfiguredDdsTopic serves
         * as the subordinate that stores and manages information about an
         * application callback that should be invoked in response to incoming
         * DDS traffic.
         * 
         * \author Rolando J. Nieves
         * \date 2012-09-26 16:39:27
         */
        class SampleCallback
        {

        public:
            static void destroy(SampleCallback *aCallback)
            { delete aCallback; }

            /**
             * \brief Establish the End of the Virtual Destructor Chain
             */
            virtual ~SampleCallback()
            {

            }

            /**
             * \brief Abstract Method Meant to Invoke Application Provided Callbacks
             *
             * \param aSample DDS Topic sample that arrived and precipitated
             *                the callback.
             */
            virtual void operator()(typename TopicConfig::SampleType* aSample) =0;

        };

        /**
         * \brief Initialize All DDS Infrastructure Required to Manage a DDS Topic
         *
         * The primary constructor for the \c ConfiguredDdsTopic performs all
         * the steps required for the host application to transact on the DDS
         * topic represented by the configuration class provided as template
         * input.
         *
         * \param runLoop \c CoreKit::RunLoop that will host this topic's
         *                \c CoreKit::InputSource facade.
         * \param theParticip \c BasicDdsParticipant to which this DDS topic
         *                    belongs.
         * \param topicName Name for the topic. The argument is optional and,
         *                  if not specified, will be assumed to be
         *                  \c TopicConfig::TopicName
         * \param cfExpression Content filter expression to use when creating a
         *                     content filtered topic (optional).
         * \param throughputCapValue maximum number of samples per second
         * \throws DdsErrorException Thrown if either registration of the
         *         topic type or creation of the topic object fail.
         */
        ConfiguredDdsTopic(
            CoreKit::RunLoop *runLoop,
            BasicDdsParticipant *theParticip,
            std::string const& topicName = TopicConfig::TopicName,
            std::string const& cfExpression = "",
            unsigned throughputCapValue = TopicConfig::MaxSamplesPerSecond
        )
        : BaseDdsTopic(runLoop, topicName),
          m_theParticip(theParticip),
          m_throughputCap(throughputCapValue),
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
          m_dataReader(nullptr),
          m_dataWriter(nullptr),
          m_cfTopic(nullptr),
          m_topic(nullptr),
          m_listenerPtr(nullptr)
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
          m_dataReader(TopicConfig::ReaderType::_nil()),
          m_dataWriter(TopicConfig::WriterType::_nil()),
          m_cfTopic(DDS::ContentFilteredTopic::_nil()),
          m_topic(DDS::Topic::_nil()),
          m_listenerPtr(DDS::DataReaderListener::_nil())
#endif /* DDS_PROVIDER */
        {
            typename TopicConfig::TypeSupportType *SampleTypeSupport = new typename TopicConfig::TypeSupportType();
            DDS::ReturnCode_t ddsRetCode = DDS::RETCODE_OK;

            if (nullptr == theParticip)
            {
                throw std::runtime_error("ConfiguredDdsTopic construction. Null participant.");
            }

            if (pthread_mutex_init(&m_readMutex, nullptr) != 0)
            {
                throw CoreKit::OsErrorException("pthread_mutex_init", errno);
            }

            ddsRetCode = SampleTypeSupport->register_type(m_theParticip->appParticipant(), TopicConfig::TypeNameAlias);
            if (ddsRetCode != DDS::RETCODE_OK)
            {
                throw DdsErrorException("register_type", ddsRetCode);
            }
            m_topic = m_theParticip->appParticipant()->create_topic(
                this->topicName().c_str(),
                TopicConfig::TypeNameAlias,
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
                DDS::TOPIC_QOS_DEFAULT,
                nullptr,
                0
#elif defined(DDS_PROVIDER) && (DDS_PROVIDER == DDS_PROVIDER_OSPL)
                TOPIC_QOS_DEFAULT,
                nullptr,
                DDS::STATUS_MASK_NONE
#elif defined(DDS_PROVIDER) && (DDS_PROVIDER == DDS_PROVIDER_ODDS)
                TOPIC_QOS_DEFAULT,
                DDS::TopicListener::_nil(),
                OpenDDS::DCPS::DEFAULT_STATUS_MASK
#endif /* DDS_PROVIDER */
                );
            if (!m_topic)
            {
                throw DdsErrorException("create_topic", DDS::RETCODE_ERROR);
            }
            if (!cfExpression.empty())
            {
                m_cfTopic = m_theParticip->appParticipant()->create_contentfilteredtopic(
                        this->topicName().c_str(),
                        m_topic, cfExpression.c_str(),
                        DDS::StringSeq());
                if (!m_cfTopic)
                {
                    m_theParticip->appParticipant()->delete_topic(m_topic);
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
                    m_topic = nullptr;
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
                    m_topic = DDS::Topic::_nil();
#endif /* DDS_PROVIDER */
                    throw DdsErrorException("create_contentfilteredtopic", DDS::RETCODE_ERROR);
                }
            }
            theParticip->addTopic(this);
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
            m_listenerPtr = this;
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
            // Store the listener pointer in a type that is compatible with the
            // set_listener() method call, but permanently bumping the 
            // reference count to avoid accidental deletion.
            m_listenerPtr = DDS::DataReaderListener::_duplicate(this);
#endif /* DDS_PROVIDER */
        }

        /**
         * \brief Tear Down All the DDS Topic Infrastructure Owned by This Instance
         */
        virtual ~ConfiguredDdsTopic()
        {
            if (m_dataReader)
            {
                m_dataReader->set_listener(
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
                    nullptr, 0
#elif defined(DDS_PROVIDER) && (DDS_PROVIDER == DDS_PROVIDER_OSPL)
                    DDS::DataReaderListener::_nil(), DDS::STATUS_MASK_NONE
#elif defined(DDS_PROVIDER) && (DDS_PROVIDER == DDS_PROVIDER_ODDS)
                    DDS::DataReaderListener::_nil(), OpenDDS::DCPS::DEFAULT_STATUS_MASK
#endif /* DDS_PROVIDER */
                    );
                m_theParticip->subscriber()->delete_datareader(m_dataReader);
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
                m_dataReader = nullptr;
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
                m_dataReader = TopicConfig::ReaderType::_nil();
#endif /* DDS_PROVIDER */
            }
            if (m_dataWriter)
            {
                m_theParticip->publisher()->delete_datawriter(m_dataWriter);
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
                m_dataWriter = nullptr;
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
                m_dataWriter = TopicConfig::WriterType::_nil();
#endif /* DDS_PROVIDER */
            }
            if (m_cfTopic)
            {
                m_theParticip->appParticipant()->delete_contentfilteredtopic(m_cfTopic);
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
                m_cfTopic = nullptr;
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
                m_cfTopic = DDS::ContentFilteredTopic::_nil();
#endif /* DDS_PROVIDER */
            }
            if (m_topic)
            {
                m_theParticip->appParticipant()->delete_topic(m_topic);
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
                m_topic = nullptr;
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
                m_topic = DDS::Topic::_nil();
#endif /* DDS_PROVIDER */
            }
            std::for_each(m_callbacks.begin(), m_callbacks.end(), &SampleCallback::destroy);
            m_callbacks.clear();
            std::for_each(m_cachedSamples.begin(), m_cachedSamples.end(), &ConfiguredDdsTopic::deleteSample);
            m_cachedSamples.clear();
            pthread_mutex_destroy(&m_readMutex);
            m_listenerPtr = nullptr;
        }

        /**
         * \brief Enable this \c ConfiguredDdsTopic Instance to Receive Topic Samples.
         *
         * The \c createReader() method in \c ConfiguredDdsTopic provides a
         * concrete implementation to the abstract method defined in
         * \c BaseDdsTopic, fulfilling the intent as laid out by the
         * \c BaseDdsTopic class by creating a \c DDS::DataReader to service
         * incoming samples on the DDS topic this instance manages.
         *
         * \throws DdsErrorException Thrown if creation of the
         *         \c DDS::DataReader failed.
         */
        virtual void createReader(
            bool reliable=false,
            unsigned historyDepth=0u
        )
        {
            DDS::DataReaderQos drQos;
            std::string readerName;

            if (m_dataReader)
            {
                throw CoreKit::PreconditionNotMetException("Data Reader Not Configured");
            }
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
            m_theParticip->subscriber()->get_default_datareader_qos(&drQos);
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
            m_theParticip->subscriber()->get_default_datareader_qos(drQos);
#endif /* DDS_PROVIDER */
            drQos.history.kind = DDS::KEEP_LAST_HISTORY_QOS;
            if (0u == historyDepth)
            {
                drQos.history.depth = TopicConfig::DefaultHistoryDepth;
            }
            else
            {
                drQos.history.depth = historyDepth;
            }
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
            /*
             * This is a CoreDX-only extension, but a very useful one at that.
             */
            readerName = TopicConfig::TopicName + std::string("DR");
            memset(drQos.entity_name.value, 0x00, sizeof(drQos.entity_name.value));
            strncpy(drQos.entity_name.value, readerName.c_str(), sizeof(drQos.entity_name.value) - 1u);
#endif /* DDS_PROVIDER == DDS_PROVIDER_COREDX */
            if (reliable)
            {
                drQos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;
            }
            else
            {
                drQos.reliability.kind = DDS::BEST_EFFORT_RELIABILITY_QOS;
            }
            if (!m_cfTopic)
            {
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
                DDS::DataReader *dataReader = nullptr;
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
                DDS::DataReader_var dataReader = DDS::DataReader::_nil();
#endif /* DDS_PROVIDER */
                dataReader = m_theParticip->subscriber()->create_datareader(
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
                        static_cast<DDS::TopicDescription*>(m_topic),
                        drQos,
                        nullptr,
                        0
#elif defined(DDS_PROVIDER) && (DDS_PROVIDER == DDS_PROVIDER_OSPL)
                        m_topic,
                        drQos,
                        nullptr,
                        DDS::STATUS_MASK_NONE
#elif defined(DDS_PROVIDER) && (DDS_PROVIDER == DDS_PROVIDER_ODDS)
                        m_topic,
                        drQos,
                        DDS::DataReaderListener::_nil(),
                        OpenDDS::DCPS::DEFAULT_STATUS_MASK
#endif /* DDS_PROVIDER */
                        );

#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
                m_dataReader = (typename TopicConfig::ReaderPtrType)dataReader;
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
                m_dataReader = TopicConfig::ReaderType::_narrow(dataReader);
#endif /* DDS_PROVIDER */
            }
            else
            {
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
                DDS::DataReader *dataReader = nullptr;
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
                DDS::DataReader_var dataReader = DDS::DataReader::_nil();
#endif /* DDS_PROVIDER */
                dataReader = m_theParticip->subscriber()->create_datareader(
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
                        static_cast<DDS::TopicDescription*>(m_cfTopic),
                        drQos,
                        nullptr,
                        0
#elif defined(DDS_PROVIDER) && (DDS_PROVIDER == DDS_PROVIDER_OSPL)
                        m_cfTopic,
                        drQos,
                        nullptr,
                        DDS::STATUS_MASK_NONE
#elif defined(DDS_PROVIDER) && (DDS_PROVIDER == DDS_PROVIDER_ODDS)
                        m_cfTopic,
                        drQos,
                        DDS::DataReaderListener::_nil(),
                        OpenDDS::DCPS::DEFAULT_STATUS_MASK
#endif /* DDS_PROVIDER */
                        );

#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
                m_dataReader = (typename TopicConfig::ReaderPtrType)dataReader;
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
                m_dataReader = TopicConfig::ReaderType::_narrow(dataReader);
#endif /* DDS_PROVIDER */
            }
            if (!m_dataReader)
            {
                std::stringstream errorMsg;
                errorMsg << "Could not create DataReader for topic " << TopicConfig::TopicName;
                throw DdsErrorException(errorMsg.str(), DDS::RETCODE_ERROR);
            }
        }

        /**
         * \brief Enable this \c ConfiguredDdsTopic Instance to Send Topic Samples
         *
         * The \c createWriter() method in \c ConfiguredDdsTopic provides a
         * concrete implementation to the abstract method defined in
         * \c BaseDdsTopic, fulfilling the intent as laid out by the
         * \c BaseDdsTopic class by creating a \c DDS::DataWriter to process
         * outgoing topic samples on the the DDS topic this instance manages.
         *
         * \throws DdsErrorException Thrown if creation of the
         *         \c DDS::DataWriter instance failed.
         */
        virtual void createWriter(bool reliable=false, unsigned historyDepth=0u)
        {
            DDS::DataWriterQos dwQos;
            std::string writerName;

            if (m_dataWriter)
            {
                throw CoreKit::PreconditionNotMetException("Data Writer Not Configured");
            }
            m_theParticip->publisher()->get_default_datawriter_qos(
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
                &dwQos
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
                dwQos
#endif /* DDS_PROVIDER */
                );
            dwQos.history.kind = DDS::KEEP_LAST_HISTORY_QOS;
            if (0u == historyDepth)
            {
                dwQos.history.depth = TopicConfig::DefaultHistoryDepth;
            }
            else
            {
                dwQos.history.depth = historyDepth;
            }

#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
            /*
             * This is a CoreDX-only extension, but a very useful one at that.
             */
            writerName = TopicConfig::TopicName + std::string("DW");
            memset(dwQos.entity_name.value, 0x00, sizeof(dwQos.entity_name.value));
            strncpy(dwQos.entity_name.value, writerName.c_str(), sizeof(dwQos.entity_name.value) -1u);
#endif /* DDS_PROVIDER == DDS_PROVIDER_COREDX */
            if (reliable)
            {
                dwQos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;
            }
            else
            {
                dwQos.reliability.kind = DDS::BEST_EFFORT_RELIABILITY_QOS;
            }
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
            DDS::DataWriter *
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
            DDS::DataWriter_var
#endif /* DDS_PROVIDER */
                dataWriter;
            dataWriter = m_theParticip->publisher()->create_datawriter(
                m_topic,
                dwQos,
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
                nullptr,
                0
#elif defined(DDS_PROVIDER) && (DDS_PROVIDER == DDS_PROVIDER_OSPL)
                nullptr,
                DDS::STATUS_MASK_NONE
#elif defined(DDS_PROVIDER) && (DDS_PROVIDER == DDS_PROVIDER_ODDS)
                DDS::DataWriterListener::_nil(),
                OpenDDS::DCPS::DEFAULT_STATUS_MASK
#endif /* DDS_PROVIDER */
                );

#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
            m_dataWriter = (typename TopicConfig::WriterPtrType)dataWriter;
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
            m_dataWriter = TopicConfig::WriterType::_narrow(dataWriter);
#endif /* DDS_PROVIDER */
            if (!m_dataWriter)
            {
                std::stringstream errorMsg;
                errorMsg << "Could not create DataWriter for topic " << TopicConfig::TopicName;
                throw DdsErrorException(errorMsg.str(), DDS::RETCODE_ERROR);
            }
        }

        /**
         * \brief Invoke the \c inputAvailableFrom() Method in This Class.
         *
         * The \c fireCallback() method, usually called by the
         * \c CoreKit::RunLoop class, simply forwards the call to the
         * \c inputAvailableFrom() method of this class.
         */
        virtual void fireCallback()
        {
            this->inputAvailableFrom(this);
        }

        /**
         * \brief Handle Incoming DDS Samples for the Topic Managed by this Instance
         *
         * The \c inputAvailableFrom() method augments the implementation
         * provided by the \c BaseDdsTopic class by processing all samples
         * in the sample cache and distributing them to interested application
         * entities via registered callbacks. After all samples are
         * distributed, they are disposed of.
         *
         * \param inputSource This instance type-casted to
         *                    \c CoreKit::InputSource
         */
        virtual void inputAvailableFrom(CoreKit::InputSource* inputSource)
        {
            BaseDdsTopic::inputAvailableFrom(inputSource);
            typename TopicConfig::SampleType *aSample = nullptr;
            size_t cacheSize = 0u;

            do
            {
                {
                    CoreKit::BlockGuard guard(&m_readMutex);
                    cacheSize = m_cachedSamples.size();
                    if (cacheSize > 0)
                    {
                        if (!m_throughputCap.canSend())
                        {
                            break;
                        }
                        aSample = m_cachedSamples.front();
                        m_cachedSamples.erase(m_cachedSamples.begin());
                    }
                }

                if (aSample != nullptr)
                {
                    this->distributeSample(aSample);
                    ConfiguredDdsTopic::deleteSample(aSample);
                    aSample = nullptr;
                }
            } while (cacheSize > 0u);
            
            if (cacheSize > 0u)
            {
                this->indicateInput();
            }
        }

        /**
         * \brief Acquire the Next DDS Sample for the Topic This Instance Manages
         *
         * The \c takeNextSample() method may be used by applications to read
         * incoming samples without using the \c CoreKit::RunLoop or an
         * interrupt-driven approach.
         *
         * \note
         * Using this method while the \c ConfiguredDdsTopic instance is
         * registered with a \c CoreKit::RunLoop may result in unpredictable
         * results.
         *
         * \param theSampleRet Topic sample instance that will be populated
         *                     with the contents of the incoming topic sample.
         *
         * \return \c true if the sample was received successfully; \c false
         *         otherwise.
         *
         * \throws CoreKit::PreconditionNotMetException if this instance was
         *         not previously configured to receive topic samples via the
         *         \c createReader() method.
         */
        bool takeNextSample(typename TopicConfig::SampleType *theSampleRet)
        {
            DDS::ReturnCode_t ddsRetCode = DDS::RETCODE_OK;

            if (!m_dataReader)
            {
                throw CoreKit::PreconditionNotMetException("Data Reader Configured");
            }

            if (nullptr == theSampleRet)
            {
                return false;
            }

#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX) || (DDS_PROVIDER == DDS_PROVIDER_ODDS)
            DDS::SampleInfo sampleInfo;
            ddsRetCode = m_dataReader->take_next_sample(theSampleRet,
# if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
            &sampleInfo
# elif defined(DDS_PROVIDER) && (DDS_PROVIDER == DDS_PROVIDER_ODDS)
            sampleInfo
# endif /* DDS_PROVIDER */
            );

            return ((DDS::RETCODE_OK == ddsRetCode) && (sampleInfo.valid_data > 0));
#elif defined(DDS_PROVIDER) && (DDS_PROVIDER == DDS_PROVIDER_OSPL)
            // OpenSplice Communtiy Edition v6.4 does not implement
            // take_next_sample()
            DDS::SampleInfoSeq sampleInfos;
            typename TopicConfig::SampleSeqType samples;
            ddsRetCode = m_dataReader->take(
                samples,
                sampleInfos,
                1,
                DDS::NOT_READ_SAMPLE_STATE,
                DDS::ANY_VIEW_STATE,
                DDS::ANY_INSTANCE_STATE);
            if ((DDS::RETCODE_OK == ddsRetCode) && sampleInfos[0].valid_data)
            {
                *theSampleRet = samples[0];
            }
            m_dataReader->return_loan(samples, sampleInfos);

            return ((DDS::RETCODE_OK == ddsRetCode) && sampleInfos[0].valid_data);
#endif /* DDS_PROVIDER */
        }

        /**
         * \brief Transmit a Topic Sample Germane to this \c ConfiguredDdsTopic Instance
         *
         * The \c writeSample() method takes a topic sample and submits it to
         * the DDS implementation for immediate transmission. Actual
         * transmission time may vary depending on the DDS implementation.
         *
         * \param theSample DDS topic sample that is to be transmitted.
         *
         * \return \c true if the sample was transmitted successfully;
         *         \c false otherwise
         *
         * \throws CoreKit::PreconditionNotMetException if this instance was
         *         not previously configured to transmit data via a call to the
         *         \c createWriter() method.
         */
        bool writeSample(typename TopicConfig::SampleType *theSample)
        {
            DDS::ReturnCode_t ddsRetCode = DDS::RETCODE_OK;

            if (!m_dataWriter)
            {
                throw CoreKit::PreconditionNotMetException("Data Writer Configured");
            }

            if (nullptr == theSample)
            {
                return false;
            }

            ddsRetCode = m_dataWriter->write(
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
                theSample,
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
                *theSample,
#endif /* DDS_PROVIDER */
                DDS::HANDLE_NIL);

            return (DDS::RETCODE_OK == ddsRetCode);
        }

        /**
         * \brief Establish the Required Infrastructure To Asynchronously Receive DDS Samples
         *
         * The \c listenForSamples() method does all the work required to
         * implement asynchronous reception of DDS data via a
         * \c CoreKit::RunLoop scheduler. This method augments the
         * implementation in the \c BaseDdsTopic class by creating a DDS data
         * reader (if one had not already been created) and associating this
         * object instance as a DDS data reader listener with the data reader.
         */
        virtual void listenForSamples()
        {
            BaseDdsTopic::listenForSamples();

            if (!m_dataReader)
            {
                createReader();
            }

            m_dataReader->set_listener(
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
                m_listenerPtr,
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
                m_listenerPtr,
#endif /* DDS_PROVIDER */
                DDS::DATA_AVAILABLE_STATUS);
        }

        /**
         * \brief Tear Down Infrastructure used to Asynchronously Receive DDS Samples
         *
         * The \c stopListening() method deregisters this instance as a
         * \c DDS::DataReaderListener for the data reader that may be present
         * and associated with this DDS topic. The base implementation in
         * \c BaseDdsTopic finishes the job.
         */
        virtual void stopListening()
        {
            if (m_dataReader)
            {
                m_dataReader->set_listener(
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
                    nullptr, 0
#elif defined(DDS_PROVIDER) && (DDS_PROVIDER == DDS_PROVIDER_OSPL)
                    nullptr, DDS::STATUS_MASK_NONE
#elif defined(DDS_PROVIDER) && (DDS_PROVIDER == DDS_PROVIDER_ODDS)
                    DDS::DataReaderListener::_nil(), OpenDDS::DCPS::DEFAULT_STATUS_MASK
#endif /* DDS_PROVIDER */
                    );
            }

            BaseDdsTopic::stopListening();
        }

        virtual size_t querySampleLostCount()
        {
            size_t result = 0u;
            DDS::SampleLostStatus sampleLostStatus;
            DDS::ReturnCode_t ddsRetCode = DDS::RETCODE_OK;

            if (m_dataReader)
            {
                ddsRetCode = m_dataReader->get_sample_lost_status(
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
                    &sampleLostStatus
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
                    sampleLostStatus
#endif /* DDS_PROVIDER */
                );
                result = (ddsRetCode == DDS::RETCODE_OK ? sampleLostStatus.total_count : 0u);
            }

            return result;
        }

        /**
         * \brief Accept a New Callback Object to Invoke When New Samples Arrive
         *
         * The \c registerCallback() method takes and \b adopts the
         * \c SampleCallback object provided as input, invoking it whenever any
         * \b new samples arrive over the network.
         *
         * \param theCallback \c SampleCallback object to include in the
         *                    distribution list for new samples.
         */
        void registerCallback(SampleCallback *theCallback)
        {
            if (theCallback != nullptr)
            {
                m_callbacks.push_back(theCallback);
            }
        }

        /**
         * \brief Handle Incoming DDS Topic Samples
         *
         * The \c on_data_available() method is defined by the
         * \c DDS::DataReaderListener interface, and it is called by a
         * \c DDS::DataReader when new samples are available for reading.
         *
         * \param theReader \c DDS::DataReader that holds samples available
         *                  for reading.
         */
        virtual void on_data_available(
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
            DDS::DataReader*
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
            DDS::DataReader_ptr
#endif /* DDS_PROVIDER */
            theReader)
#if defined(DDS_PROVIDER) && (DDS_PROVIDER == DDS_PROVIDER_ODDS)
            throw (CORBA::SystemException)
#endif /* DDS_PROVIDER == DDS_PROVIDER_ODDS */
        {
            DDS::ReturnCode_t ddsRetCode = DDS::RETCODE_OK;
            typename TopicConfig::SampleSeqType samples;
            DDS::SampleInfoSeq sampleInfos;
            size_t cacheSize = 0u;

            if (theReader != m_dataReader)
            {
                return;
            }

            ddsRetCode = m_dataReader->take(
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
                &samples,
                &sampleInfos,
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
                samples,
                sampleInfos,
#endif /* DDS_PROVIDER */
                /* maximum sample count */100,
                DDS::NOT_READ_SAMPLE_STATE,
                DDS::ANY_VIEW_STATE,
                DDS::ANY_INSTANCE_STATE
            );
            if (ddsRetCode != DDS::RETCODE_OK)
            {
                if (ddsRetCode != DDS::RETCODE_NO_DATA && m_theParticip->useLog())
                {
                    *m_theParticip->useLog() << CoreKit::AppLog::LL_WARNING
                                             << "Could not take() "
                                             << TopicConfig::TopicName
                                             << " samples."
                                             << CoreKit::EndLog;
                }
            }
            else
            {
                CoreKit::BlockGuard guard(&m_readMutex);
                for (int sampleIdx = 0;
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
                     sampleIdx < samples.size();
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
                     sampleIdx < samples.length();
#endif /* DDS_PROVIDER */
                     sampleIdx++)
                {
                    if (
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
                        sampleInfos[sampleIdx]->valid_data
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
                        sampleInfos[sampleIdx].valid_data
#endif /* DDS_PROVIDER */
                        )
                    {
                        m_cachedSamples.push_back(
                            new typename TopicConfig::SampleType(
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
                                *samples[sampleIdx]
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
                                samples[sampleIdx]
#endif /* DDS_PROVIDER */
                            )
                        );
                    }
                }
                while (m_cachedSamples.size() > TopicConfig::DefaultHistoryDepth)
                {
                    ConfiguredDdsTopic::deleteSample(m_cachedSamples.front());
                    m_cachedSamples.erase(m_cachedSamples.begin());
                }
                cacheSize = m_cachedSamples.size();
            }
#if !defined(DDS_PROVIDER) || (defined(DDS_PROVIDER_COREDX) && (DDS_PROVIDER == DDS_PROVIDER_COREDX))
            m_dataReader->return_loan(&samples, &sampleInfos);
#elif defined(DDS_PROVIDER) && defined(DDS_PROVIDER_OSPL) && (DDS_PROVIDER == DDS_PROVIDER_OSPL)
            m_dataReader->return_loan(samples, sampleInfos);
#endif /* defined(DDS_PROVIDER) */
            if (cacheSize > 0u)
            {
                this->indicateInput();
            }
        }

        /**
         * \brief Obtain A Mutable Handle to the Underlying DDS::Topic Object
         *
         * \return Pointer to the \c DDS::Topic object managed by this
         *         instance.
         */
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
        DDS::Topic *const
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
        DDS::Topic_ptr
#endif /* DDS_PROVIDER */
            useDdsTopic()
        {
            return m_topic;
        }

        inline bool isContentFiltered() const
        {
            return (bool)m_cfTopic;
        }

#if defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
        virtual void on_requested_deadline_missed(DDS::DataReader_ptr reader, DDS::RequestedDeadlineMissedStatus const& status)
# if (DDS_PROVIDER == DDS_PROVIDER_ODDS)
            throw(CORBA::SystemException)
# endif /* DDS_PROVIDER */
        {}
        virtual void on_requested_incompatible_qos(DDS::DataReader_ptr reader, DDS::RequestedIncompatibleQosStatus const& status)
# if (DDS_PROVIDER == DDS_PROVIDER_ODDS)
            throw(CORBA::SystemException)
# endif /* DDS_PROVIDER */
        {}
        virtual void on_sample_rejected(DDS::DataReader_ptr reader, DDS::SampleRejectedStatus const& status)
# if (DDS_PROVIDER == DDS_PROVIDER_ODDS)
            throw(CORBA::SystemException)
# endif /* DDS_PROVIDER */
        {}
        virtual void on_liveliness_changed(DDS::DataReader_ptr reader, DDS::LivelinessChangedStatus const& status)
# if (DDS_PROVIDER == DDS_PROVIDER_ODDS)
            throw(CORBA::SystemException)
# endif /* DDS_PROVIDER */
        {}
        virtual void on_subscription_matched(DDS::DataReader_ptr reader, DDS::SubscriptionMatchedStatus const& status)
# if (DDS_PROVIDER == DDS_PROVIDER_ODDS)
            throw(CORBA::SystemException)
# endif /* DDS_PROVIDER */
        {}
        virtual void on_sample_lost(DDS::DataReader_ptr reader, DDS::SampleLostStatus const& status)
# if (DDS_PROVIDER == DDS_PROVIDER_ODDS)
            throw(CORBA::SystemException)
# endif /* DDS_PROVIDER */
        {}
#endif /* DDS_PROVIDER */
    private:
        /**
         * \brief Cache for Samples Received from the DDS Implementation
         */
        std::vector<typename TopicConfig::SampleType*> m_cachedSamples;
        /**
         * \brief Collection of Callback Objects Interested in Incoming Samples
         */
        std::vector<SampleCallback*> m_callbacks;
        /**
         * \brief \c DDS::DataReader Instance used to Accept Topic Samples
         */
        typename TopicConfig::ReaderPtrType m_dataReader;
        /**
         * \brief \c DDS::DataWriter Instance Used to Dispatch Topic Samples
         */
        typename TopicConfig::WriterPtrType m_dataWriter;
        /**
         * \brief \c DDS:Topic Instance Wrapped by this \c ConfiguredDdsTopic Instance
         */
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
        DDS::Topic*
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
        DDS::Topic_var
#endif /* DDS_PROVIDER */
            m_topic;
        /**
         * \brief \c DDS::ContentFilteredTopic Instance Wrapped by this \c ConfiguredDdsTopic Instance
         */
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
        DDS::ContentFilteredTopic*
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
        DDS::ContentFilteredTopic_var
#endif /* DDS_PROVIDER */
            m_cfTopic;
        /**
         * \brief \c BasicDdsParticipant (or derivative) Instance Hosting This Topic
         */
        BasicDdsParticipant *m_theParticip;

        /**
         * \brief Synchronizaton object used to coordinate access to the sample cache.
         */
        pthread_mutex_t m_readMutex;

        /**
         * \brief Convenience pointer container for our DataReaderListener
         */
#if !defined(DDS_PROVIDER) || (DDS_PROVIDER == DDS_PROVIDER_COREDX)
        DDS::DataReaderListener *m_listenerPtr;
#elif defined(DDS_PROVIDER) && ((DDS_PROVIDER == DDS_PROVIDER_OSPL) || (DDS_PROVIDER == DDS_PROVIDER_ODDS))
        DDS::DataReaderListener_ptr m_listenerPtr;
#endif /* DDS_PROVIDER */

        /**
         * \brief Throughput enforcing monitor
         */
        ThroughputCap m_throughputCap;

        /**
         * \brief Send a DDS Sample to All Registered Callbacks
         *
         * The \c distributeSample() convenience method iterates through all
         * callbacks registered with this instance, sending to each one the
         * sample provided as input. This method is primarily used by the
         * \c inputAvailableFrom() method in this class.
         *
         * \param theSample DDS sample to distribute to all registered
         *                  callbacks.
         */
        void distributeSample(typename TopicConfig::SampleType *theSample)
        {
            std::for_each(m_callbacks.begin(), m_callbacks.end(),
                    std::bind2nd(std::mem_fun(&SampleCallback::operator()), theSample));
        }

        /**
         * \brief Destory a DDS Sample
         *
         * The \c deleteSample() convenience method is used to clean up samples
         * in the cache kept by this object instance. It is primarily used by
         * the \c inputAvailableFrom() method.
         *
         * \param theSample DDS sample to destroy
         */
        static void deleteSample(typename TopicConfig::SampleType *theSample)
        {
            delete theSample;
        }

    };

}
#endif // !defined(EA_F3240A50_DB91_4c1e_99FE_0AB644DDC500__INCLUDED_)
