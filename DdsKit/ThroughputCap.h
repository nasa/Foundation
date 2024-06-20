/**
 * \file ThroughputCap.h
 * \brief Contains the definition of the \c DdsKit::ThroughputCap class.
 * \date 2019-08-05 09:15:08
 * \author Rolando J. Nieves
 */

#ifndef _FOUNDATION_DDSKIT_THROUGHPUTCAP_H_
#define _FOUNDATION_DDSKIT_THROUGHPUTCAP_H_

namespace DdsKit
{

/**
 * \brief Defines a cap on the number of samples to be published per second
 * \author Rolando J. Nieves
 * \date 2019-08-05
 */
class ThroughputCap
{
private:
    unsigned m_cap;
    double m_lastSentTime;
    unsigned m_sentCount;

public:
    /**
     * \brief Constructor
     * \param cap the throughput cap
     */
    explicit ThroughputCap(unsigned cap);

    /**
     * \brief Destructor
     */
    virtual ~ThroughputCap();

    /**
     * \brief Determine if the cap has been reached
     * \return true if permitted to send, false if cap has been reached
     */
    bool canSend();
};

} // end namespace DdsKit

#endif /* !_FOUNDATION_DDSKIT_THROUGHPUTCAP_H_ */

// vim: set ts=4 sw=4 expandtab:
