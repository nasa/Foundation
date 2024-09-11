/**
 * \file IsoDdsKit/TopicCollection.cpp
 * \brief Contains the implementation for the \c IsoDdsKit::TopicCollection class.
 * \author Rolando J. Nieves
 * \date 2024-08-21 15:04:05
 */

#include "TopicCollection.hpp"


namespace IsoDdsKit
{

void
TopicCollection::clear()
{
    auto iterator = m_topicStorage.begin();

    while (iterator != m_topicStorage.end())
    {
        (*iterator).second.close();
        m_topicStorage.erase(iterator);
        iterator = m_topicStorage.begin();
    }
}

} // end namespace IsoDdsKit

// vim: set ts=4 sw=4 expandtab:
