/**
 * \file IsoDdsKit/ValueDistribution.hpp
 * \brief Contains the definition of the \c IsoDdsKit::ValueDistribution class.
 * \author Rolando J. Nieves
 * \date 2024-08-14 14:48:18
 */

#ifndef _FOUNDATION_ISODDSKIT_VALUEDISTRIBUTION_HPP_
#define _FOUNDATION_ISODDSKIT_VALUEDISTRIBUTION_HPP_

#include <functional>
#include <map>
#include <memory>
#include <utility>

#include <IsoDdsKit/BasicRandomIdSource.hpp>


namespace IsoDdsKit
{

template< typename ValueType >
class ValueDistribution
{
public:
    using IdSourcePtr = std::shared_ptr< UniqueIdSource >;

private:
    using Listener = std::function< void (ValueType const&) >;
    using ListenerMap = std::map< int, Listener >;

    IdSourcePtr m_listenerIdSource;
    ListenerMap m_listeners;

public:
    ValueDistribution();

    ValueDistribution(IdSourcePtr listenerIdSource);

    virtual ~ValueDistribution();

    template< typename ListenerCallable >
    int addListener(ListenerCallable&& callable);

    void emit(ValueType const& sample);

    void removeAllListeners();

    void removeListener(int listenerId);
};


template< typename ValueType >
ValueDistribution< ValueType >::ValueDistribution()
{
    m_listenerIdSource = std::make_shared< BasicRandomIdSource >();
}


template< typename ValueType >
ValueDistribution< ValueType >::ValueDistribution(IdSourcePtr listenerIdSource):
    m_listenerIdSource(listenerIdSource)
{}


template< typename ValueType >
ValueDistribution< ValueType >::~ValueDistribution()
{
    m_listeners.clear();
}


template< typename ValueType >
template< typename ListenerCallable >
int
ValueDistribution< ValueType >::addListener(ListenerCallable&& callable)
{
    constexpr int MAX_ID_GEN_TRIES = 100;

    int listenerId = m_listenerIdSource->next();
    int tries = 0;
    while ((m_listeners.find(listenerId) != m_listeners.end()) && (tries < MAX_ID_GEN_TRIES))
    {
        listenerId = m_listenerIdSource->next();
        tries++;
    }
    m_listeners.emplace(std::make_pair(listenerId, std::forward< ListenerCallable >(callable)));

    return listenerId;
}


template< typename ValueType >
void
ValueDistribution< ValueType >::emit(ValueType const& sample)
{
    for (auto const& [listenerId, callable] : m_listeners)
    {
        callable(sample);
    }
}


template< typename ValueType >
void
ValueDistribution< ValueType >::removeAllListeners()
{
    m_listeners.clear();
}


template< typename ValueType >
void
ValueDistribution< ValueType >::removeListener(int listenerId)
{
    auto listenerEntry = m_listeners.find(listenerId);
    if (listenerEntry != m_listeners.end())
    {
        m_listeners.erase(listenerEntry);
    }
}

}
#endif /* !_FOUNDATION_ISODDSKIT_VALUEDISTRIBUTION_HPP_ */

// vim: set ts=4 sw=4 expandtab:
