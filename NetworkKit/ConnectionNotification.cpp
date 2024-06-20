/**
 * \file      ConnectionNotification.cpp
 * \brief     Definition of ConnectionNotification class
 * \date      2013-11-14
 * \author    Ryan O'Farrell
 */

#include "ConnectionNotification.h"

namespace NetworkKit
{

ConnectionNotification::ConnectionNotification(
		TcpSocket * const theSocket,
		const ConnectionStates::ConnectionStatesEnum theState) :
		socket(theSocket), state(theState)
{

}

ConnectionNotification::ConnectionNotification(
		ConnectionNotification const & other) :
		socket(other.socket), state(other.state)
{
}

std::ostream & operator<<(std::ostream &os, const ConnectionNotification& p)
{
	os << "Connection notification: ";
	os << "\t Socket: " << p.socket->getSockFd();
	os << "\t State: ";
	switch(p.state)
	{
	case ConnectionStates::CONNECTED:
		os << "CONNECTED" ;
		break;
	case ConnectionStates::DISCONNECTED:
		os << "DISCONNECTED" ;
		break;
    case ConnectionStates::PENDING:
		os << "PENDING" ;
        break;
	}
	return os;
}

} /* namespace NetworkKit */
