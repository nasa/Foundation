/**
 * \file AppLog.cpp
 * \brief Implementation of the AppLog Class
 * \date 2012-09-25 11:48:27
 * \author Rolando J. Nieves
 */

#include <sys/types.h>
#include <unistd.h>
#include <cstdio>
#include <iostream>
#if defined(HAVE_SYSLOG_H) && (HAVE_SYSLOG_H == 1)
# include <syslog.h>
#endif /* defined(HAVE_SYSLOG_H) && (HAVE_SYSLOG_H == 1) */

#include <CoreKit/SystemTime.h>

#include "AppLog.h"

using std::string;
using std::ends;
using std::clog;
using std::endl;
using CoreKit::AppLog;

#define RF_AL_MAX_LOG_FIELD_SIZE (64u)
const CoreKit::AppLog::End CoreKit::EndLog = CoreKit::AppLog::End();

#if defined(HAVE_SYSLOG) && (HAVE_SYSLOG == 1)
/**
 * \brief Convert an \c AppLog Severity Level to a Syslog Severity Level
 *
 * The \c logLeveltoSyslogPriority() local function accepts an \c AppLog
 * severity level and returns an equivalent severity level suitable for use
 * with the \c syslog() library call.
 *
 * \param logLevel \c AppLog severity level to convert.
 *
 * \return \c syslog() severity level that corresponds to the provided
 *         \c AppLog severity level; \c LOG_DEBUG should there be no
 *         appropriate \c syslog() severity level.
 */
static int logLevelToSyslogPriority(AppLog::Level logLevel)
{
	int result = LOG_DEBUG;

	switch (logLevel)
	{
	case AppLog::LL_DEBUG:
		result = LOG_DEBUG;
		break;
	case AppLog::LL_INFO:
		result = LOG_INFO;
		break;
	case AppLog::LL_WARNING:
		result = LOG_WARNING;
		break;
	case AppLog::LL_ERROR:
	default:
		result = LOG_ERR;
		break;
	}

	return result;
}
#endif /* defined(HAVE_SYSLOG) && (HAVE_SYSLOG == 1) */

/**
 * \brief Convert an \c AppLog Severity Level to a Character String
 *
 * The \c logLevelToString() local function converts an \c AppLog severity
 * level value and produces a human-readable character string that describes
 * the provided severity level. This local function is primarily used when the
 * host operating system and/or C-language library does not support \c syslog()
 *
 * \param logLevel \c AppLog severity level to convert to a character string.
 *
 * \return Human readable character string describing the provided severity
 *         level.
 */
static string logLevelToString(AppLog::Level logLevel)
{
	string result = "<unknown>";

	switch(logLevel)
	{
	case AppLog::LL_DEBUG:
		result = "DEBUG";
		break;
	case AppLog::LL_INFO:
		result = "INFORMATION";
		break;
	case AppLog::LL_WARNING:
		result = "WARNING";
		break;
	case AppLog::LL_ERROR:
		result = "ERROR";
		break;
	default:
		break;
	}

	return result;
}


/*
 * This seems redundant, but the C++ compiler demands that template
 * specializations be defined within the same namespace as they are
 * declared; the fact that there's a "using" statement above does not abate
 * this requirement.
 */
namespace CoreKit {
template<>
AppLog& AppLog::operator << <AppLog::Level> (AppLog::Level logLevel)
{
	m_logLevel = logLevel;

	return *this;
}


template<>
AppLog& AppLog::operator << <AppLog::End> (AppLog::End endMark)
{
	m_currentLogMsg << ends;
#if defined(HAVE_SYSLOG) && (HAVE_SYSLOG == 1)
    if (m_logLevel >= m_minLevel)
    {
        syslog(logLevelToSyslogPriority(m_logLevel), "%s", m_currentLogMsg.str().c_str());
    }
#endif /* defined(HAVE_SYSLOG) && (HAVE_SYSLOG == 1) */

    if (m_doStdErr && (m_logLevel >= m_minLevel))
    {
        string log_time;
        clog
            << "["
            << SystemTime::secsToIsoTstamp(SystemTime::now(), log_time)
            << "] ["
            << m_appName
            << "] ["
            << getpid()
            << "] ["
            << logLevelToString(m_logLevel)
            << "]: "
            << m_currentLogMsg.str()
            << endl;
    }
    m_currentLogMsg.str("");
    m_currentLogMsg.clear();

    return *this;
}
}


AppLog::AppLog(std::string const& appName, bool doStdErr)
: m_appName(appName), m_logLevel(AppLog::LL_DEBUG), m_minLevel(AppLog::LL_DEBUG), m_doStdErr(doStdErr)
{
#if defined(HAVE_SYSLOG) && (HAVE_SYSLOG == 1)
	int options = LOG_PID;
	openlog(m_appName.c_str(), options, LOG_USER);
#endif /* defined(HAVE_SYSLOG) && (HAVE_SYSLOG == 1) */
}


AppLog::~AppLog()
{
#if defined(HAVE_SYSLOG) && (HAVE_SYSLOG == 1)
	closelog();
#endif /* defined(HAVE_SYSLOG) && (HAVE_SYSLOG == 1) */
}



string CoreKit::format(string const& formatStr, float floatVal)
{
	char fmtDest[RF_AL_MAX_LOG_FIELD_SIZE];

	snprintf(fmtDest, RF_AL_MAX_LOG_FIELD_SIZE, formatStr.c_str(), floatVal);

	return string(fmtDest);
}


string CoreKit::format(string const& formatStr, double doubleVal)
{
	char fmtDest[RF_AL_MAX_LOG_FIELD_SIZE];

	snprintf(fmtDest, RF_AL_MAX_LOG_FIELD_SIZE, formatStr.c_str(), doubleVal);

	return string(fmtDest);
}


string CoreKit::format(string const& formatStr, int intVal)
{
	char fmtDest[RF_AL_MAX_LOG_FIELD_SIZE];

	snprintf(fmtDest, RF_AL_MAX_LOG_FIELD_SIZE, formatStr.c_str(), intVal);

	return string(fmtDest);
}


std::string CoreKit::format(std::string const& formatStr, uint32_t uint32Val)
{
	char fmtDest[RF_AL_MAX_LOG_FIELD_SIZE];

	snprintf(fmtDest, RF_AL_MAX_LOG_FIELD_SIZE, formatStr.c_str(), uint32Val);

	return string(fmtDest);
}


std::string CoreKit::format(std::string const& formatStr, int16_t int16Val)
{
	char fmtDest[RF_AL_MAX_LOG_FIELD_SIZE];

	snprintf(fmtDest, RF_AL_MAX_LOG_FIELD_SIZE, formatStr.c_str(), int16Val);

	return string(fmtDest);
}


std::string CoreKit::format(std::string const& formatStr, uint16_t uint16Val)
{
	char fmtDest[RF_AL_MAX_LOG_FIELD_SIZE];

	snprintf(fmtDest, RF_AL_MAX_LOG_FIELD_SIZE, formatStr.c_str(), uint16Val);

	return string(fmtDest);
}


std::string CoreKit::format(std::string const& formatStr, uint8_t byteVal)
{
	char fmtDest[RF_AL_MAX_LOG_FIELD_SIZE];

	snprintf(fmtDest, RF_AL_MAX_LOG_FIELD_SIZE, formatStr.c_str(), byteVal);

	return string(fmtDest);
}
