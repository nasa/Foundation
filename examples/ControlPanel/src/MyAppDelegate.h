#ifndef _MY_APP_DELEGATE_H_
#define _MY_APP_DELEGATE_H_

#include <CoreKit/CoreKit.h>
#include <DdsKit/DdsKit.h>

#include <CommandConfig.h>
#include <EngineeringDataConfig.h>

#include <CommandDispatch.h>
#include <FrontPanel.h>
#include <UserInputSource.h>

class MyAppDelegate : public CoreKit::AppDelegate, public CoreKit::InterruptListener
{
public:
    MyAppDelegate();

    virtual ~MyAppDelegate();

    /**
     * \brief Configure any command-line arguments the application will accept.
     *
     * \param theApp - Pointer to the application object that should receive the
     *                 command-line argument configuration.
     */
    void configureCmdLineArgs(CoreKit::Application *theApp);

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
    std::shared_ptr< CoreKit::EventInputSource > m_cmdEvent;
    std::shared_ptr< CommandDispatch > m_commandDispatch;
    DdsKit::ConfiguredDdsTopic< CommandConfig > *m_commandTopic;
    DDS::DomainId_t m_ddsDomainId;
    DdsKit::ConfiguredDdsTopic< EngineeringDataConfig > *m_engrDataTopic;
    std::shared_ptr< FrontPanel > m_frontPanel;
    std::unique_ptr< DdsKit::BasicDdsParticipant > m_participant;
    std::shared_ptr< UserInputSource > m_userInput;
};

#endif /* _MY_APP_DELEGATE_H_ */

// vim: set ts=4 sw=4 expandtab:

