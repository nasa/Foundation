#ifndef _MY_APP_DELEGATE_H_
#define _MY_APP_DELEGATE_H_

#include <CoreKit/CoreKit.h>
#include <IsoDdsKit/IsoDdsKit.h>

#include <MyIsoSoftwareBus.h>

#include <CommandDispatch.h>
#include <FrontPanel.h>
#include <UserInputSource.h>

class MyAppDelegate : public IsoDdsKit::DdsEnabledAppDelegate, public CoreKit::InterruptListener
{
public:
    MyAppDelegate();

    virtual ~MyAppDelegate();

    /**
     * \brief Initialize the application supporting infrastructure.
     *
     * \param theApp - Pointer to the application object providing the core
     *                 infrastructure that must be customized.
     */
    virtual void applicationDidInitialize(CoreKit::Application *theApp) override;

    /**
     * \brief Spin up the application infrastructure for operations.
     *
     * \param theApp - Pointer to the application object with the infrastructure
     *                 that is ready to start.
     */
    virtual void applicationStarting(CoreKit::Application *theApp) override;

    /**
     * \brief Spin down the application infrastructure from operations.
     *
     * \param theApp - Pointer to the application object with the infrastructure
     *                 that should be spun down.
     */
    virtual void applicationWillTerminate(CoreKit::Application *theApp) override;

    /**
     * \brief Release all resources used by application infrastructure.
     *
     * \param theApp - Pointer to the application object with the infrastructure
     *                 using the resources that must be released.
     */
    virtual void applicationDidTerminate(CoreKit::Application *theApp) override;

    void onCommandRequest(std::string commandStatement);
    
private:
    using CommandWriterContextPtr = std::shared_ptr< IsoDdsKit::WriterContext< MyIsoSoftwareBus::Command > >;
    using EngrDataReaderContextPtr = std::shared_ptr< IsoDdsKit::ReaderContext< MyIsoSoftwareBus::EngineeringData > >;

    std::shared_ptr< CoreKit::EventInputSource > m_cmdEvent;
    std::shared_ptr< CommandDispatch > m_commandDispatch;
    CommandWriterContextPtr m_commandWriterContext;
    EngrDataReaderContextPtr m_engrDataReaderContext;
    std::shared_ptr< FrontPanel > m_frontPanel;
    std::shared_ptr< UserInputSource > m_userInput;
};

#endif /* _MY_APP_DELEGATE_H_ */

// vim: set ts=4 sw=4 expandtab:

