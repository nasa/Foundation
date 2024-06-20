/**
 * \file TargetedSampleCallbackWithPredT.h
 * \date 2013-03-11
 * \author Chad A. Chamberlin
 */

#ifndef TARGETEDSAMPLECALLBACKWITHPREDT_H_
#define TARGETEDSAMPLECALLBACKWITHPREDT_H_

#include <stdlib.h>
#include <DdsKit/TargetedSampleCallback.h>

namespace DdsKit
{

    /**
     * \brief Callback object bound to a conditional predicate.
     *
     * This callback object will not forward any calls that don't satisfy the
     * provided predicate. As of this writing, this only works with
     * \c DataPointFrame samples (i.e., sample types which bear a \c srcSubsys
     * field).
     */
	template<class TargetType, class TopicConfig, class PredicateType>
	class TargetedSampleCallbackWithPredT : public TargetedSampleCallback<TargetType, TopicConfig>
	{

	public:
        /**
         * \brief Preserve all details pertaining to the callback.
         *
         * \param callbackTarget Callable that will be invoked.
         * \param config Topic configuration type.
         * \param conditionPredicate Predicate used to evaluate whether to
         *                           invoke the callable.
         */
		TargetedSampleCallbackWithPredT(TargetType callbackTarget, TopicConfig config, PredicateType conditionPredicate)
		: TargetedSampleCallback<TargetType,TopicConfig>(callbackTarget), m_conditionPredicate(conditionPredicate)
		{

		}

        /**
         * \brief Destructor.
         */
		virtual ~TargetedSampleCallbackWithPredT() = default;

        /**
         * \brief Receive the callback request.
         *
         * Invokes the predicate provided at construction time to decide
         * whether to forward the call or not.
         *
         * \param aSample Pointer to the sample associated with the callback.
         */
		virtual void operator()(typename TopicConfig::SampleType* aSample)
		{

			if ((aSample != nullptr) && (m_conditionPredicate( aSample->srcSubsys ) == true))
			{
				TargetedSampleCallback<TargetType,TopicConfig>::operator()(aSample);
			}
		}

	private:

		PredicateType m_conditionPredicate;

	};


    /**
     * \brief Predicate class for the callback.
     *
     * As of this writing, this expects to compare an \c unsigned \c short
     * value, as those used in \c DataPointFrame samples.
     */
	struct SubsystemMatch
	{
	public:

        /**
         * \brief Subsystem value to compare against.
         */
		unsigned short subsystem;

        /**
         * \brief Initialize the subsystem value to compare against.
         *
         * \param theSubsystem Subsystem value to compare against.
         */
		inline explicit SubsystemMatch(const unsigned short theSubsystem) : subsystem(theSubsystem) {};

        /**
         * \brief Copy constructor.
         *
         * \param other Reference to the source in the copy operation.
         */
		inline SubsystemMatch(SubsystemMatch const& other) : subsystem(other.subsystem) {};

        /**
         * \brief Evaluate whether the subsystem matches our expected value.
         *
         * \param theSubsystem Subsystem value to compare.
         *
         * \return \c true if the value provided as input is the same as that
         *         configured at construction time; \c false otherwise.
         */
		inline bool operator()(const unsigned short theSubsystem) { return theSubsystem == subsystem; };
	};

/**
 * \brief Factory function for predicated sample callbacks.
 *
 * \note The caller assumes responsibility for the memory allocated in this
 * function.

 * \param config Topic configuration.
 * \param callbackTarget Callable to invoke.
 * \param conditionPredicate Comparison predicate to use.
 *
 * \return New predicated sample callback instance.
 */
template<class TargetType, class TopicConfig, class PredicateType>
TargetedSampleCallback<TargetType, TopicConfig>* newTargetedSampleCallbackWithPred(TopicConfig config, TargetType callbackTarget, PredicateType conditionPredicate)
{
	return new TargetedSampleCallbackWithPredT<TargetType,TopicConfig,PredicateType>(callbackTarget, config, conditionPredicate);
}


}

#endif /* TARGETEDSAMPLECALLBACKWITHPREDT_H_ */
