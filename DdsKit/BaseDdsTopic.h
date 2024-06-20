/**
 * \file BaseDdsTopic.h
 * \brief Definition of the BaseDdsTopic Class
 * \date 2012-09-26 17:15:19
 * \author Rolando J. Nieves
 */
#if !defined(EA_7507E3D8_12D6_4459_8BB2_28DF148557EB__INCLUDED_)
#define EA_7507E3D8_12D6_4459_8BB2_28DF148557EB__INCLUDED_

#include <CoreKit/RunLoop.h>
#include <CoreKit/InterruptListener.h>
#include <CoreKit/InputSource.h>
#include <string>

namespace DdsKit
{
/**
    * \brief Base Class for All DDS Topics Collaborating with \c BasicDdsParticipant
    *
    * The \c BaseDdsTopic defines the basic interface and services required to
    * integrate interaction with a DDS topic within the context of a
    * \c BasicDdsParticipant and, consequently, with a \c CoreKit::RunLoop
    * instance.\par
    *
    * The \c BaseDdsTopic class acts as a \c CoreKit::InputSource with a given
    * \c CoreKit::RunLoop instance, meaning it is able to provide a run loop
    * with asynchronous notifications when topic samples arrive at the
    * application's DDS interface.\par
    *
    *
    * \c BaseDdsTopic uses the \c eventfd() Linux facilities to produce a
    * file descriptor that will exhibit input activity whenever DDS
    * traffic is received.\par
    *
    * This class, although available, is not meant to be used directly by
    * application developers. Developers should instead rely on the
    * \c ConfiguredDdsTopic class.
    * 
    * \author Rolando J. Nieves
    * \date 2012-09-26 15:43:25
    */
class BaseDdsTopic : public CoreKit::InputSource, public CoreKit::InterruptListener
{

public:
    /**
        * \brief Initialize Instance Fields to Nominal Values
        *
        * \param runLoop \c CoreKit::RunLoop instance that this
        *                \c BaseDdsTopic instance should integrate with
        *                regarding DDS input activity.
        * \param topicName Name for the new topic.
        */
    BaseDdsTopic(CoreKit::RunLoop* runLoop, std::string const& topicName);
    /**
        * \brief Dispose of All DDS Entities Owned by This \c BaseDdsTopic Instance
        *
        * The destructor for the \c BaseDdsTopic class deactivates and
        * disposes of any listeners that may have been created to monitor for
        * DDS input, then disposes of all \c DDS::Topic, \c DDS::DataReader,
        * and \c DDS::DataWriter instances owned by this class.
        */
    virtual ~BaseDdsTopic();

    /**
        * \brief Abstract Method Meant to Create \c DDS::DataReader Instances
        *
        * The \c createReader() method is defined for classes deriving from
        * \c BaseDdsTopic to implement. The expectation from derived classes
        * is that after this method successfully completes, the
        * \c BaseDdsTopic instance is able to read DDS samples. \par
        *
        * The method is not implemented in this class because \c BaseDdsTopic
        * does not know enough information on its own to correctly create
        * the infrastructure required to support a \c DDS::DataReader.\par
        *
        * The method does not return any \c DDS::DataReader instances created
        * by derived classes, giving derived classes freedom regarding how to
        * manage any created DDS entities.
        */
    virtual void createReader(bool reliable=false, unsigned historyDepth=0u) =0;
    /**
        * \brief Abstract Method Meant to Create \c DDS::DataWriter Instances
        *
        * The \c createWriter() method is defined for classes deriving from
        * \c BaseDdsTopic to implement. The expectation from derived classes
        * is that after this method successfully completes, the
        * \c BaseDdsTopic instance is able to write DDS samples. \par
        *
        * The method is not implemented in this class becase \c BaseDdsTopic
        * does not know enough information on its own to correctly create the
        * infrastructure required to support a \c DDS::DataWriter.\par
        *
        * The method does not return any \c DDS::DataWriter instances created
        * by derived classes, giving derived classes freedom regarding how to
        * manage any created DDS entities.
        */
    virtual void createWriter(bool reliable=false, unsigned historyDepth=0u) =0;
    /**
        * \brief Obtain The File Descriptor Used to Indicate DDS Activity
        *
        * The \c fileDescriptor() method, defined in \c CoreKit::InputSource,
        * provides a \c CoreKit::RunLoop the file descriptor that should be
        * monitored in order to detect DDS input activity on the DDS topic
        * managed by this instance.
        *
        * \return File descriptor that will exhibit input activity whenever
        *         topic samples arrive.
        */
    virtual int fileDescriptor() const;
    /**
        * \brief Provide a Type-casted Pointer to This Instance
        *
        * The \c interruptListener() method, originally defined in the
        * \c CoreKit::InputSource class, is used by this class to provide
        * a type-casted version of itself for use whenever DDS input does
        * arrive.
        *
        * \return \c BaseDdsTopic instance type-casted as
        *         \c CoreKit::InterruptListener .
        */
    virtual InterruptListener* interruptListener() const;
    /**
        * \brief Service the \c eventfd() Object Used to Indicate Input
        *
        * The \c inputAvailableFrom() method is called whenever DDS topic
        * samples arrive for the topic managed by this \c BaseDdsTopic
        * instance. The implementation in this class services the \c eventfd()
        * object used to integrate with \c CoreKit::RunLoop, but does not
        * retrieve any DDS data. Derived classes are expected to do the rest
        * of the work.
        *
        * \param theSource This \c BaseDdsTopic instance.
        */
    virtual void inputAvailableFrom(CoreKit::InputSource *theSource);
    /**
     * \brief Start Listening for DDS Topic Samples
     *
     * The \c listenForSamples() method creates the infrastructure required
     * to integrate with a \c CoreKit::RunLoop instace by creating an
     * \c eventfd() object. The expectation is that derived classes
     * implement the rest of the infrastructure required to listen for
     * incoming DDS samples.
     */
    virtual void listenForSamples();
    /**
        * \brief Tear Down Infrastructure Used to Listen for DDS Traffic
        *
        * The \c stopListening() method closes down and destroys the
        * \c eventfd() object used to integrate with \c CoreKit::RunLoop.
        * The expectation is that derived classes implement the rest of the
        * tear down by deactivating and destroying the rest of the
        * infrastucture required to receive DDS samples
        * (e.g., \c DDS::DataReaderListener ).
        */
    virtual void stopListening();

    /**
     * \brief Query the number of samples that DDS has reported as lost
     * \return number of lost samples
     */
    virtual size_t querySampleLostCount();

    /**
     * \brief Get the topic name
     * \return the topic name
     */
    inline std::string const& topicName() const { return m_topicName; }
    
protected:
    /**
        * \brief \c CoreKit::RunLoop Instance Integrated with This \c BaseDdsTopic Instance
        */
    CoreKit::RunLoop *m_runLoop;

    /**
        * \brief Convenience Routine Used to Force Activity on \c eventfd() Object
        *
        * \c BaseDdsTopic derived classes are encouraged to use the
        * \c indicateInput() method to cause the associated
        * \c CoreKit::RunLoop instance to schedule work for this instance.
        * This method sends a 64-bit integer value of 1uLL via the
        * \c eventfd() object to indicate input activity. Read the
        * \c eventfd() manual page for more information.
        */
    void indicateInput();

private:
    /**
        * \brief File Descriptor Used to Store the \c eventfd() Object Used by This Instance
        */
    int m_eventFd;
    /**
        * \brief Name for the topic.
        */
    std::string m_topicName;

};

}
#endif // !defined(EA_7507E3D8_12D6_4459_8BB2_28DF148557EB__INCLUDED_)

// vim: set ts=4 sw=4 expandtab:
