/**
 * \file EventInputSource.h
 * \brief Contains the definition of the \c EventInputSource class.
 * \date 2019-05-15 16:44:58
 * \author Rolando J. Nieves
 */

#ifndef _FOUNDATION_COREKIT_EVENTINPUTSOURCE_H_
#define _FOUNDATION_COREKIT_EVENTINPUTSOURCE_H_

#include <CoreKit/factory.h>
#include <CoreKit/InputSource.h>

namespace CoreKit
{

/**
 * \brief Model \c CoreKit::RunLoop input source that leverages Linux \c eventfd()
 */
class EventInputSource : public InputSource
{
    RF_CK_FACTORY_COMPATIBLE(EventInputSource);

private:
    InterruptListener *m_intrListener;
    int m_eventFd;

public:
    /**
     * \brief Main constructor.
     *
     * Save the interrupt listener dependency injection for future stimulus
     * handling.
     *
     * \param[in] intrListener - Pointer to the listener that will receive
     *            callbacks off stimuli generated by this input source.
     */
    explicit EventInputSource(InterruptListener *intrListener);

    /**
     * \brief Destructor.
     */
    virtual ~EventInputSource();

    /**
     * \brief Access the underlying file descriptor allocated to this input source.
     *
     * \return File descriptor associated with this input source.
     */
    virtual int fileDescriptor() const override;

    /**
     * \brief Access the interrupt listener dependency injected at construction time.
     *
     * \return Pointer to interrupt listener associated with this instance.
     */
    virtual InterruptListener* interruptListener() const override;

    /**
     * \brief Deliver stimuli notification to interrupt listener.
     */
    virtual void fireCallback() override;

    /**
     * \brief Generate a stimuli for processing by the host \c CoreKit::RunLoop
     *
     * The corresponding stimulus callback is not invoked right away, but rather
     * at the next run loop iteration.
     */
    virtual void assertEvent();
};

} // end namespace CoreKit

#endif /* !_FOUNDATION_COREKIT_EVENTINPUTSOURCE_H_ */

// vim: set ts=4 sw=4 expandtab:
