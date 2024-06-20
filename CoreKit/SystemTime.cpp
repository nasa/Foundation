/**
 * \file SystemTime.cpp
 * \brief Implementation of the SystemTime Class
 * \date 2014-06-03 13:54:22
 * \author Rolando J. Nieves
 */

#include <stdint.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cmath>

#include "SystemTime.h"

#define RF_ST_TIME_STRING_SIZE (40u)
#define RF_ST_NANOS_PER_USEC (1000)
#define RF_ST_NANOS_PER_MSEC (1000000.0)
#define RF_ST_USECS_PER_SEC (1000000.0)
#define RF_ST_NANOS_PER_SEC (1000000000.0)
#define RF_ST_MSEC_PER_SEC (1000.0)
#define RF_ST_SECS_PER_HOUR (3600.0)
#define RF_ST_DBL_EXP_BIAS (1023LL)

using std::string;
using std::isfinite;

namespace CoreKit
{

static unsigned highestBitPosOf(uint32_t value)
{
	static const unsigned NIBBLE_LOOKUP[] = {
		0u, 1u, 2u, 2u, 3u, 3u, 3u, 3u, 4u, 4u, 4u, 4u, 4u, 4u, 4u, 4u };
	unsigned result = 0u;
	
	//
	// This set of "if" and "else" statements is basically a binary tree that 
	// speeds up the search for the highest significant bit in a 32-bit value.
	// Thus, finding the bit should at worst case cost 3 "if" comparisons, an
	// integer addition, a bitwise AND, and a bit shift.
	//
	if (value > 0x0000FFFF)
	{
		// Between 0x00010000 and 0xFFFFFFFF
		if (value > 0x00FFFFFF)
		{
			// Between 0x01000000 and 0xFFFFFFFF
			if (value > 0x0FFFFFFF)
			{
				// Between 0x10000000 and 0xFFFFFFFF
				result = 28u;
				result += NIBBLE_LOOKUP[(value & 0xF0000000) >> 28u];
			}
			else
			{
				// Between 0x01000000 and 0x0FFFFFFFF
				result = 24u;
				result += NIBBLE_LOOKUP[(value & 0x0F000000) >> 24u];
			}
		}
		else
		{
			// Between 0x00010000 and 0x00FFFFFF
			if (value > 0x000FFFFF)
			{
				// Between 0x00100000 and 0x00FFFFFF
				result = 20u;
				result += NIBBLE_LOOKUP[(value & 0x00F00000) >> 20u];
			}
			else
			{
				// Between 0x00010000 and 0x000FFFFF
				result = 16u;
				result += NIBBLE_LOOKUP[(value & 0x000F0000) >> 16u];
			}
		}
	}
	else
	{
		// Between 0x00000000 and 0x0000FFFF
		if (value > 0x000000FF)
		{
			// Between 0x00000100 and 0x0000FFFF
			if (value > 0x00000FFF)
			{
				// Between 0x00001000 and 0x0000FFFF
				result = 12u;
				result += NIBBLE_LOOKUP[(value & 0x0000F000) >> 12u];
			}
			else
			{
				// Between 0x00000100 and 0x00000FFF
				result = 8u;
				result += NIBBLE_LOOKUP[(value & 0x00000F00) >> 8u];
			}
		}
		else
		{
			// Between 0x00000000 and 0x000000FF
			if (value > 0x0000000F)
			{
				// Between 0x00000010 and 0x000000FF
				result = 4u;
				result += NIBBLE_LOOKUP[(value & 0x000000F0) >> 4u];
			}
			else
			{
				// Between 0x00000000 and 0x0000000F
				result = NIBBLE_LOOKUP[value];
			}
		}
	}
	
	return result;
}


static void parseTimeStringTrailer(string const& trailer, suseconds_t& millis, double& tzOffset)
{
	enum { 
		MILLIS_OR_TZ, 
		MILLIS_DOT, 
		MILLIS_DIGIT, 
		TZ_LEADER, 
		TZ_DIGIT } parseState = MILLIS_OR_TZ;
	string::size_type currentPos = 0u;
	char millisStr[4];
	char *millisDigit = &millisStr[0];
	char tzStr[4];
	char *tzChar = &tzStr[0];
	
	memset(&millisStr[0], 0x00, sizeof(millisStr));
	memset(&tzStr[0], 0x00, sizeof(tzStr));
	
	while ((currentPos != string::npos) && (currentPos < trailer.length()))
	{
		switch(parseState)
		{
		case MILLIS_OR_TZ:
			if (trailer[currentPos] == '.')
			{
				parseState = MILLIS_DOT;
			}
			else if ((trailer[currentPos] == '+') ||
				(trailer[currentPos] == '-') ||
				(trailer[currentPos] == 'Z'))
			{
				parseState = TZ_LEADER;
			}
			else
			{
				currentPos = string::npos;
			}
			break;
		case MILLIS_DOT:
			if ((trailer.length() - currentPos) >= 4u)
			{
				currentPos++;
				parseState = MILLIS_DIGIT;
			}
			else
			{
				memset(&millisStr[0], 0x00, sizeof(millisStr));
				currentPos = string::npos;
			}
			break;
		case MILLIS_DIGIT:
			if (isdigit(trailer[currentPos]))
			{
				(*millisDigit) = trailer[currentPos];
				millisDigit++;
				currentPos++;
			}
			else
			{
				memset(&millisStr[0], 0x00, sizeof(millisStr));
				currentPos = string::npos;
			}
			
			if (millisDigit - &millisStr[0] == 3)
			{
				parseState = TZ_LEADER;
			}
			break;
		case TZ_LEADER:
			if (trailer[currentPos] == 'Z')
			{
				strncpy(tzStr, "0", sizeof(tzStr) - 1u);
				currentPos = string::npos;
			}
			else if ((trailer[currentPos] == '-') ||
				(trailer[currentPos] == '+'))
			{
				(*tzChar) = trailer[currentPos];
				currentPos++;
				tzChar++;
				parseState = TZ_DIGIT;
			}
			else
			{
				currentPos = string::npos;
				memset(&tzStr[0], 0x00, sizeof(tzStr));
			}
			break;
		case TZ_DIGIT:
			if (isdigit(trailer[currentPos]))
			{
				(*tzChar) = trailer[currentPos];
				currentPos++;
				tzChar++;
			}
			else
			{
				currentPos = string::npos;
				memset(&tzStr[0], 0x00, sizeof(tzStr));
			}
			
			if (tzChar - &tzStr[0] == 3)
			{
				currentPos = string::npos;
			}
			break;
		}
	}
	
	if (strlen(millisStr))
	{
		millis = strtol(millisStr, NULL, 10);
	}
	
	if (strlen(tzStr))
	{
		tzOffset = strtod(tzStr, NULL);
	}
	else
	{
		tzOffset = INFINITY;
	}
}


double SystemTime::now()
{
	timespec currTime;
	
	memset(&currTime, 0x00, sizeof(currTime));
	return SystemTime::secsFromTimespec(SystemTime::nowAsTimespec(currTime));
}


double SystemTime::secsFromTimespec(timespec const& ts)
{
	unsigned secHighBitPos = 0u;
	unsigned fSecHighBitPos = 0u;
	int64_t exponent = RF_ST_DBL_EXP_BIAS;
	double result = 0.0;
	uint64_t *scratchpad = reinterpret_cast<uint64_t*>(&result);
	
	if ((ts.tv_sec == 0) && (ts.tv_nsec == 0))
	{
		return result;
	}
	
	//
	// Quantizing nanoseconds (10^-9) into 2^-20 seconds (~usecs):
	// Let (f) be the resulting fractions of a second, (n) be the original
	// nanoseconds:
	// (f) = (n) * ((10^-9) / (2^-20))
	// (f) = (n) * ((2^20) / (10^9))
	// (f) = (n) * ((2^20) / (5^9 * 2^9))
	// (f) = (n) * ((2^11 * 2^9) / (5^9 * 2^9))
	// (f) = (n) * (2^11 / 5^9)
	// 5^9 => 1953125
	// In C++ code:
	// f = (n << 11) / 1953125;
	//
	(*scratchpad) = (static_cast<uint64_t>(ts.tv_nsec) << 11uLL) / 1953125uLL;
	
	if (ts.tv_sec > 0)
	{
		//
		// OR into the mantissa the seconds, while leaving the least 
		// significant 20 bits occupied by the fractional seconds alone.
		//
		(*scratchpad) |= static_cast<uint64_t>(ts.tv_sec) << 20uLL;
		
		secHighBitPos = highestBitPosOf(ts.tv_sec);
		
		//
		// 33 comes from the 32 bits reserved for the seconds plus the implied
		// top bit in the normalized mantissa
		//
		(*scratchpad) <<= (33uLL - secHighBitPos);
		
		//
		// Exponent bias moves up by the distance between the 32nd bit and the
		// highest significant bit in the seconds
		//
		exponent += (secHighBitPos - 1u);
	}
	else
	{
		fSecHighBitPos = highestBitPosOf((*scratchpad));
		
		//
		// 53 comes from the 52 bits allocated to the mantissa plus the implied
		// bit.
		//
		(*scratchpad) <<= (53uLL - fSecHighBitPos);
		
		//
		// 21 comes from the 20 bits reserved for the fractional seconds plus
		// the implied top bit in the normalized mantissa. Exponent bias moves
		// down by the distance between the implied top bit in the mantissa and
		// the highest significant bit in the fractional seconds.
		//
		exponent -= (21u - fSecHighBitPos);
	}
	
	(*scratchpad) &= 0x000FFFFFFFFFFFFFuLL;
	(*scratchpad) |= (exponent & 0x7FF) << 52LL;
	
	return result;
}


timespec const& SystemTime::secsToTimespec(double secs, timespec& ts)
{
	double microsecs = 0.0;
	ts.tv_sec = static_cast<time_t>(secs);
	
	microsecs = secs - static_cast<double>(ts.tv_sec);
	microsecs *= RF_ST_USECS_PER_SEC;
	ts.tv_nsec = static_cast<suseconds_t>(microsecs) * RF_ST_NANOS_PER_USEC;
	
	return ts;
}


double SystemTime::secsFromIsoTstamp(string const& isoTstamp)
{
	static const string BASIC_PLACEHOLDER("YYYY-mm-ddTHH:MM:SS");
	struct tm timeStruct;
	string basicStr;
	string trailerStr;
	double tzOffset = INFINITY;
	suseconds_t millisecs = 0;
	double result = 0.0;
	time_t tzCorrection = 0;
	time_t nowEpoch = 0;
	long ourGmtTzOffset = 0;
	
	memset(&timeStruct, 0x00, sizeof(timeStruct));
	
	basicStr = isoTstamp.substr(0, BASIC_PLACEHOLDER.length());
	
	//
	// This step is necessary because according to the man page, strptime()
	// does not initialize all values of the struct tm data structure with
	// appropriate values. localtime_r() does.
	//
	tzset();
	nowEpoch = time(&nowEpoch);
	localtime_r(&nowEpoch, &timeStruct);
	ourGmtTzOffset = timeStruct.tm_gmtoff;
	strptime(basicStr.c_str(), "%FT%T", &timeStruct);
	
	//
	// Parse out the milliseconds and/or the timezone
	//
	if (isoTstamp.length() > BASIC_PLACEHOLDER.length())
	{
		trailerStr = isoTstamp.substr(BASIC_PLACEHOLDER.length());
		parseTimeStringTrailer(trailerStr, millisecs, tzOffset);
	}
	
	result = static_cast<double>(mktime(&timeStruct));
	if (isfinite(tzOffset))
	{
		result += ourGmtTzOffset - (tzOffset * RF_ST_SECS_PER_HOUR);
	}
	else
	{
		result += ourGmtTzOffset - timeStruct.tm_gmtoff;
	}
	result += static_cast<double>(millisecs) / RF_ST_MSEC_PER_SEC;
	
	return result; 
}


string const& SystemTime::secsToIsoTstamp(double secs, string& isoTstamp)
{
	char timeString[RF_ST_TIME_STRING_SIZE];
	char msecString[RF_ST_TIME_STRING_SIZE];
	struct tm timeStruct;
	time_t epochTime = 0;
	double millisecs = 0.0;
	
	memset(timeString, 0x00, sizeof(timeString));
    memset(msecString, 0x00, sizeof(msecString));
	memset(&timeStruct, 0x00, sizeof(timeStruct));
	
	epochTime = static_cast<time_t>(secs);
	gmtime_r(&epochTime, &timeStruct);
	
	millisecs = (secs - static_cast<double>(epochTime));
	strftime(timeString, sizeof(timeString) - 1u, "%FT%T", &timeStruct);
	snprintf(msecString, sizeof(msecString), "%.03f", millisecs);
	snprintf(&timeString[strlen(timeString)], sizeof(timeString) - 1u, "%s", 
		&msecString[1]);
	snprintf(&timeString[strlen(timeString)], sizeof(timeString) - 1u, "Z");
	
	isoTstamp = timeString;
	
	return isoTstamp;
}

} // namespace CoreKit
