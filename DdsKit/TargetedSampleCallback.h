/**
 * \file TargetedSampleCallback.h
 * \brief Definition of the TargetedSampleCallback Class
 * \date 2012-09-27 09:19:17
 * \author Rolando J. Nieves
 */
#if !defined(EA_10AE3D72_5D48_45eb_B103_08624A0F2024__INCLUDED_)
#define EA_10AE3D72_5D48_45eb_B103_08624A0F2024__INCLUDED_

#include <DdsKit/ConfiguredDdsTopic.h>

namespace DdsKit
{
	/**
	 * \brief \c DdsKit compatible callbacks targeted at a configurable object.
	 * \author Rolando J. Nieves
	 * \date 2012-09-26 16:53:13
	 */
	template<class TargetType, class TopicConfig>
	class TargetedSampleCallback : public DdsKit::ConfiguredDdsTopic<TopicConfig>::SampleCallback
	{

	public:

        /**
         * \brief Constructor
         * \param callbackTarget the callback target
         */
		explicit TargetedSampleCallback(TargetType callbackTarget)
		: m_callbackTarget(callbackTarget)
		{

		}

        /**
         * \brief Destructor
         */
		virtual ~TargetedSampleCallback()
		{

		}

        /**
         * \brief Forwards the sample to the callback target
         * \param aSample the received DDS sample
         */
		virtual void operator()(typename TopicConfig::SampleType* aSample)
		{
			m_callbackTarget(aSample);
		}

	private:

		TargetType m_callbackTarget;
	};

	/**
	 * \brief Create a \c TargetedSampleCallback instance for a specific topic.
	 * \param theConfig Copy of the topic configuration data structure. Only 
	 *                  used for automatic template argument deciphering.
	 * \param callbackTarget Callable object that the 
	 *                       \c TargetedSampleCallback instance will invoke.
	 */
	template<class TargetType, class TopicConfig>
	TargetedSampleCallback<TargetType,TopicConfig>* newCallbackForTopic(TopicConfig theConfig,
			TargetType callbackTarget)
	{
		return new TargetedSampleCallback<TargetType, TopicConfig>(callbackTarget);
	}
}
#endif // !defined(EA_10AE3D72_5D48_45eb_B103_08624A0F2024__INCLUDED_)
