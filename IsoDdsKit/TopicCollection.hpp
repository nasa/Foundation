/**
 * \file IsoDdsKit/TopicCollection.cpp
 * \brief Contains the definition of the \c IsoDdsKit::TopicCollection class.
 * \author Rolando J. Nieves
 * \date 2024-08-21 14:03:07
 */

#ifndef _FOUNDATION_ISODDSKIT_TOPICCOLLECTION_HPP_
#define _FOUNDATION_ISODDSKIT_TOPICCOLLECTION_HPP_

#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

#include <dds/domain/DomainParticipant.hpp>
#include <dds/topic/qos/TopicQos.hpp>
#include <dds/topic/AnyTopic.hpp>
#include <dds/topic/Topic.hpp>

namespace IsoDdsKit
{

class TopicCollection
{
private:
    using TopicStorage = std::map< std::string, dds::topic::AnyTopic >;

    TopicStorage m_topicStorage;

public:
    TopicCollection() = default;

    virtual ~TopicCollection() = default;

    template< typename SampleType >
    void add(dds::domain::DomainParticipant& participant, std::string const& topicName);

    template< typename SampleType >
    void add(dds::domain::DomainParticipant& participant, std::string const& topicName, dds::topic::qos::TopicQos const& qos);

    void clear();

    template< typename SampleType >
    dds::topic::Topic< SampleType > use(std::string const& topicName);
};

template< typename SampleType >
void
TopicCollection::add(dds::domain::DomainParticipant& participant, std::string const& topicName)
{
    std::stringstream errorMsg;

    if (m_topicStorage.find(topicName) != m_topicStorage.end())
    {
        errorMsg << "Attempt to create topic with already-existing name \"" << topicName << "\"";
        throw std::runtime_error(errorMsg.str());
    }

    dds::topic::Topic< SampleType > theTopic(participant, topicName);
    if (theTopic.is_nil())
    {
        errorMsg << "Could not create topic \"" << topicName << "\"";
        throw std::runtime_error(errorMsg.str());
    }

    m_topicStorage.emplace(std::make_pair(topicName, std::forward< dds::topic::AnyTopic >(theTopic)));
}

template< typename SampleType >
void
TopicCollection::add(dds::domain::DomainParticipant& participant, std::string const& topicName, dds::topic::qos::TopicQos const& qos)
{
    std::stringstream errorMsg;

    if (m_topicStorage.find(topicName) != m_topicStorage.end())
    {
        errorMsg << "Attempt to create topic with already-existing name \"" << topicName << "\"";
        throw std::runtime_error(errorMsg.str());
    }

    dds::topic::Topic< SampleType > theTopic(participant, topicName, qos);
    if (theTopic.is_nil())
    {
        errorMsg << "Could not create topic \"" << topicName << "\"";
        throw std::runtime_error(errorMsg.str());
    }

    m_topicStorage.emplace(std::make_pair(topicName, std::forward< dds::topic::AnyTopic >(theTopic)));
}

template< typename SampleType >
dds::topic::Topic< SampleType >
TopicCollection::use(std::string const& topicName)
{
    auto topicIter = m_topicStorage.find(topicName);
    if (topicIter != m_topicStorage.end())
    {
        return dds::topic::Topic< SampleType >((*topicIter).second);
    }
    else
    {
        return dds::core::null;
    }
}

} // end namespace IsoDdsKit

#endif /* !_FOUNDATION_ISODDSKIT_TOPICCOLLECTION_HPP_ */

// vim: set ts=4 sw=4 expandtab:
