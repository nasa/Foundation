#include <cstdlib>
#include <cstdio>
#include <exception>
#include <memory>

#include <CoreKit/CoreKit.h>

#include "MyAppDelegate.h"

using std::exception;
using CoreKit::G_MyApp;
using CoreKit::Application;

int main(int argc, const char **argv)
{
    int result = EXIT_SUCCESS;
    std::unique_ptr< MyAppDelegate > theDelegate;

    try
    {
        // Create the application delegate instance
        theDelegate.reset(new MyAppDelegate());

        // Create the application instance and associate it with the
        // newly-created delegate. The CoreKit-defined G_MyApp global variable
        // is used here to facilitate access to the main application object
        // across all application code. Use of the global variable, however, is
        // not mandatory.
        G_MyApp = new Application(theDelegate.get());

        // Allow the delegate a chance to configure any command-line arguments
        // it may need.
        theDelegate->configureCmdLineArgs(G_MyApp);

        // Officially initialize the application, providing it all command-line
        // arguments.
        G_MyApp->initialize("ApplicationName", argc, argv);

        // Start the application logic. This call will block until the
        // application is done.
        G_MyApp->start();
    }
    catch (exception const& ex)
    {
        fprintf(stderr, "Exception raised: %s\n", ex.what());
        result = EXIT_FAILURE;
    }

    // Dispose of the application instance pointed to by the G_MyApp global.
    delete G_MyApp;
    G_MyApp = nullptr;

    // Dispose of the application delegate instance created at startup.
    theDelegate.reset();

    return result;
}
