#ifndef _MY_APP_DELEGATE_H_
#define _MY_APP_DELEGATE_H_

#include <CoreKit/CoreKit.h>

class MyAppDelegate : public CoreKit::AppDelegate
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
};

#endif /* _MY_APP_DELEGATE_H_ */
