/**
 * \file SystemTime.h
 * \brief Definition of the SystemTime Class
 * \date 2014-06-03 13:54:22
 * \author Rolando J. Nieves
 */
#if !defined(EA_16066B4E_8B39_4060_921C_A30018B053AF__INCLUDED_)
#define EA_16066B4E_8B39_4060_921C_A30018B053AF__INCLUDED_

#include <ctime>
#include <string>

namespace CoreKit
{
/**
 * \brief Convenience methods for dealing with system time.
 * \author Rolando J. Nieves
 * \date 2014-06-03 13:23:03
 */
class SystemTime
{
public:

    /**
     * \brief Acquire the current time as a real value.
     *
     * The whole part of the real value is the seconds since the UNIX epoch,
     * while the fractions is a value quantized to 2^-20, or roughly to the
     * microsecond level.
     *
     * \note 32-bit Linux/UNIX systems will experience clock rollover on 19
     *       January 2038. 64-bit Linux/UNIX systems won't roll over until much
     *       (MUCH!) later. This middle-of-the-road approach does not roll over,
     *       but begins to lose precision sometime in the year 2242 C.E.
     *
     * \return Current time value expressed in terms of the UNIX epoch.
     */
	static double now();
	
    /**
     * \brief Acquire the current time as a structured value.
     *
     * \return Current time as a whole seconds and seconds fraction structure,
     *         expressed in terms of the UNIX epoch.
     */
	inline static timespec const& nowAsTimespec(timespec& ts)
	{ clock_gettime(CLOCK_REALTIME, &ts); return ts; }
	
    /**
     * \brief Convert a time structure to a real value.
     *
     * \param[in] ts - Reference to the structured time value.
     *
     * \return Translated time as a real value.
     */
	static double secsFromTimespec(timespec const& ts);
	
    /**
     * \brief Convert a real time value to structured value.
     *
     * \param[in] secs Time expressed as a real value in seconds since the
     *            UNIX epoch.
     * \param[in,out] ts Reference to the structured time value to update.
     *
     * \return Reference to the structured time instance after updating.
     */
	static timespec const& secsToTimespec(double secs, timespec& ts);
	
    /**
     * \brief Convert a real time value to an ISO-formatted timestamp.
     *
     * \param[in] secs Time expressed as a real value in seconds since the
     *            UNIX epoch.
     * \param[in,out] isoTstamp Character string that will be modified with the
     *                ISO-formatted timestamp.
     *
     * \return Reference to the character string object after modification.
     */
	static std::string const& secsToIsoTstamp(double secs, std::string& isoTstamp);
	
    /**
     * \brief Parse an ISO-formatted timestamp
     *
     * Convert the ISO-formatted timestamp into a real value expressed in
     * seconds since the UNIX epoch.
     *
     * \param[in] isoTstamp Character string that contains the ISO-formatted
     *            timestamp.
     *
     * \return Time expressed as a real value of seconds since the UNIX epoch.
     */
	static double secsFromIsoTstamp(std::string const& isoTstamp);
};

}
#endif // !defined(EA_16066B4E_8B39_4060_921C_A30018B053AF__INCLUDED_)
