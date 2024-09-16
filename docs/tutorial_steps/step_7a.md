# Tutorial Step 7: Software Bus Participation Basics (ISO Variant)

Several of the examples prior to this one have demonstrated, in their own way,
communication between software components. Although effective, they require a
good amount of effort from developers to properly format, emit, and receive
information. The Foundation leverages another technology to further facilitate
software component inter-connection: the Object Management Group's (OMG)
[Data Distribution Service (DDS)][omg_dds_spec].

## A (very) Brief DDS Discussion

DDS is a publish-subscribe communication fabric, where software components
openly advertise both what information they have and information they need. As
the components survey those incoming advertisements, they establish lines of
communication amongst themselves.

The information the software components exchange is formatted as specified in a
machine-ingestible description artifact written a subset of the OMG
[Interface Definition Language][omg_idl_spec]. The formatted messages are
distributed into named "topics." The aforementioned advertisements from software
components are specified in terms of these named topics. In addition to the
message structure specification, topics also include configuration parameters
that describe some of the characteristics of *how* the messages are exchanged.
Those characteristics are typically referred to as the "Quality of Service"
(QoS) associated with the topic.

DDS-enabled applications are able to exchange information in a platform and
network transparent manner. This means that application developers need not
worry about implementing conventions for exchanging multi-byte numeric values
that adapt to a platform's native multi-byte handling (big endian vs. little
endian). This also means that applications are able to exchange information in
the same way whether they are co-located on a single host, or distributed across
several hosts in a local network.

The DDS specification is implemented by entities usually referred to as
"providers." The products offered by these providers implement the specification
in terms of a concrete computer programming language. The Foundation relies
exclusively on the C++ implementation from these providers.

There are many (many!) more details regarding DDS that this example makes
reference to. If any concepts seem unfamiliar, please refer to a DDS
introductory tutorial for more information. This example relies on the
[Cyclone DDS][CycloneDDS] provider.

## The Software Bus

In Foundation, the sample and topic definitions are collectively referred to as
the "Software Bus." Although it would be possible to establish integration of
multiple application by just sharing the message structure definitions written
in IDL, the approach this example will adopt groups both the message and topic
definitions into a single library that applications can link to and leverage.

## Example Design

Unlike other tutorial steps, this example will be split into two (2) separate
projects:

* One project contains the [Software Bus](#the-software-bus) definitions and
  produces a static library and header files that can be imported into other
  projects.
  * The software bus will define two (2) message types:
    * One message type carries engineering data samples.
    * Another message type carries operator commands.
* The other project contains the Foundation application that communicates over
  the software bus. The application will have two operating modes:
  * When operating as a publisher, it will emit a topic sample at regular
    intervals.
  * When operating as a subscriber, it will report any samples it observes
    in the software bus.

## Software Bus Library Bootstrap

This is a completely new type of project, starting from a different project
template compared to the other examples. In a terminal shell session with the
current directory where all the other tutorial example project directories are
located, give the project folder the name `MyIsoSoftwareBus` and copy over the
Foundation-provided ISO software bus skeleton with the following commands (the
`FOUNDATION_ROOT` environment variable points to the Foundation root folder):

```console
$ mkdir MyIsoSoftwareBus
$ cd MyIsoSoftwareBus
$ rsync -auv ${FOUNDATION_ROOT}/examples/SampleIsoSoftwareBus/ .
```

Once copied over, the structure of the project tree should be as follows:

```
MyIsoSoftwareBus
├─cmake
│ ├─Modules
│ │ ├─CoreDX-build.cmake
│ │ ├─CycloneDDS-CXX-build.cmake
│ │ └─FindCoreDX.cmake
│ └─SampleIsoSoftwareBusConfig.cmake
├─src
│ ├─ProviderName
│ └─SampleIsoSoftwareBus.h.in
└─CMakeLists.txt
```

Each file and folder serves a different purpose. Several of them are identical
to those included in the sample application skeleton. Refer to
[Step 1](./step_1.md#bootstrap-the-project-tree) for descriptions on files not
explained here:

* `cmake/SampleIsoSoftwareBusConfig.cmake` - Contains package configuration
  entry point meant for use by applications that link against this library.
* `src/ProviderName` - Folder that could contain the code generated by the DDS
  provider when developers choose to preserve generated code. Preserving
  generated code can be useful for developers, or automated build systems, that
  need to build the library without access to the IDL compiler available.
* `src/SampleIsoSoftwareBus.h.in` - Template header file used to generate a
  usable header file that includes all provider-generated headers.

This example will base its software bus on the [Cyclone DDS][CycloneDDS]
provider, since it is the most readily available. Rename the
`src/ProviderName` directory to be `src/CycloneDDS-CXX` from the project root
folder:

```console
$ mv src/ProviderName src/CycloneDDS-CXX
```

> [!NOTE]
> The Foundation build tree that this example will use must have also been built
> with [Cyclone DDS][CycloneDDS] as its DDS provider and the ISO variant
> `IsoDdsKit`. Refer to the [Foundation build](../building.md) page for more
> information on how to select [Cyclone DDS][CycloneDDS] as the DDS provider
> and the [ISO variant `IsoDdsKit` for Foundation](../building.md#building-with-isoddskit).

## Application Project Bootstrap

Since the application will be significantly different from those created in
previous tutorial steps, it will start from the baseline application skeleton
provided by the Foundation. In a terminal shell session with the current
directory being the one containing the previously-created `MyIsoSoftwareBus`
directory, give the application project folder the name `TutorialStep7a` and
copy over the baseline skeleton with the following commands (the
`FOUNDATION_ROOT` environment variable points to the Foundation root folder):

```console
$ mkdir TutorialStep7a
$ cd TutorialStep7a
$ rsync -auv ${FOUNDATION_ROOT}/examples/SampleAppSkeleton/ .
```

## Software Bus Library Build Configuration

Build configuration involves a bit more for the software bus project than what
has been done in previous tutorial steps. The primary reason for the difference
is the fact that the project must produce both a library that applications can
link against, as well as the required configuration that the applications will
use to configure themselves for linking/importing the library in. The
application project being built in this tutorial step will leverage this
configuration to link in the software bus library. First, in the
`MyIsoSoftwareBus` project directory, modify the project metadata towards the
beginning of the `CMakeLists.txt` file as follows:

```diff
cmake_minimum_required(VERSION 3.16)

-project(SampleIsoSoftwareBus VERSION "1.0.0" LANGUAGES CXX)
+project(MyIsoSoftwareBus VERSION "1.0.0" LANGUAGES CXX)
```

Next, modify the set of source files used to build the application by updating
the `create_sb_library_targets()` function call as follows:

```diff
create_sb_library_targets(
    TARGET "${PROJECT_NAME}"
    IDL_FILES
-       "<insert IDL file path>"
+       "src/MyIsoSoftwareBus.idl"
)
```

> [!NOTE]
> All required mentions of the DDS provider to use are indirectly referred to
> via the `${WITH_DDS}` CMake variable. This will be a variable that will be
> defined when the project is built. The references are left such that
> interested readers have an easier time adapting the project to other DDS
> providers.

## Application Project Build Configuration

Similar to the other examples, this example application must be given a unique
name. Furthermore, the project dependency list must be updated to include
support for DDS. In the `TutorialStep7a` directory, edit the project metadata
in the `CMakeLists.txt` `project()` command as follows:

```diff
- project(SampleApp LANGUAGES CXX)
+ project(IsoSoftwareBusPrimer LANGUAGES CXX)
```

Then, update the project dependencies in the `find_package()` command:

```diff
-find_package(Foundation REQUIRED CoreKit)
+find_package(Foundation REQUIRED COMPONENTS CoreKit IsoDdsKit)
+find_package(MyIsoSoftwareBus REQUIRED)
```

The `IsoDdsKit` name identifies the module that brings in the ISO DDS
functionality the application will use. In addition, there's a new
`find_package()` statement that imports the software bus library being created
in this example step. The application build configuration command shows how the
`cmake` tool will find the `MyIsoSoftwareBus` package.

Finally, include the required components in the `target_link_libraries()`
command as follows:

```diff
-target_link_libraries(${PROJECT_NAME} Foundation::CoreKit)
+target_link_libraries(${PROJECT_NAME} Foundation::CoreKit Foundation::IsoDdsKit MyIsoSoftwareBus::MyIsoSoftwareBus)
```

The `target_link_libraries()` command statement folds in the `IsoDdsKit` and
`MyIsoSoftwareBus` components, identified by their fully-scoped names.

## Software Bus Message Definition

As initially enumerated in the [example design](#example-design), the software
bus library will define two (2) message structures. One structure will be
designed to carry engineering data for a fictional instrument. The other
structure will be designed to carry commands as dispatched by an operator. Add
the following content to the `src/MyIsoSoftwareBus.idl` IDL source file in the
`MyIsoSoftwareBus` directory:

```
module MyIsoSoftwareBus
{
    struct Command
    {
        string statement;
    };
    #pragma keylist Command

    struct EngineeringData
    {
        long alpha;
        long bravo;
        long charlie;
    };
    #pragma keylist EngineeringData
};
```

A complete discussion on the IDL syntax is beyond the scope of this tutorial,
but a good summary of what the file defines is as follows:

* A `Command` message type holding a single field meant to hold the action
  the target device should take.
* An `EngineeringData` message type holding all of the state acquired from
  the device instrumentation points.

> [!NOTE]
> The `#pragma keylist` pre-processor definitions are required by
> [Cyclone DDS][CycloneDDS] so that it knows the mentioned type name will be
> used as a topic sample type. Other DDS providers may have different
> requirements regarding the identification of types for topic samples.

This example will only make use of the `EngineeringData` message type. The
`Command` message type will be saved for use in future examples.

## Software Bus Template Header File

The template header file provided in the bootstrap copy of the software bus
must bear the name of the project. From the `MyIsoSoftwareBus` project
directory, change the name of the template header file as follows:

```console
$ mv src/SampleIsoSoftwareBus.h.in src/MyIsoSoftwareBus.h.in
```

## Application Main Function

There are very little changes required of the main function, just like in
[Step 1](./step_1.md). The change involves altering the application name for
logging purposes in the call to `::CoreKit::Application::initialize()`. From the
`TutorialStep7a` project folder modify the `src/main.cpp` file as follows:

```diff
- G_MyApp->initialize("ApplicationName", argc, argv);
+ G_MyApp->initialize("IsoSoftwareBusPrimer", argc, argv);
```

## Application Delegate

### Class Structure Customization

Since the logic in this example is relatively simple, it will all be contained
within the application delegate class. From the `TutorialStep7a` project folder,
modify the beginning of the `src/MyAppDelegate.h` file as follows:

```diff
#ifndef _MY_APP_DELEGATE_H_
#define _MY_APP_DELEGATE_H_

+#include <memory>
+#include <random>
+#include <string>
#include <CoreKit/CoreKit.h>
+#include <IsoDdsKit/IsoDdsKit.h>
+#include <MyIsoSoftwareBus.h>

-class MyAppDelegate : public CoreKit::AppDelegate
+class MyAppDelegate : public IsoDdsKit::DdsEnabledAppDelegate, public CoreKit::InterruptListener
{
public:
+   static std::string EngineeringDataToString(MyIsoSoftwareBus::EngineeringData const& sample);
+
    MyAppDelegate();
```

Unlike all other examples up to this point, the application delegate class for
this application derives from the `IsoDdsKit::DdsEnabledAppDelegate` class
instead of directly from the `CoreKit::AppDelegate` class.
`IsoDdsKit::DdsEnabledAppDelegate` derives from `CoreKit::AppDelegate` and
adds functionality common to all DDS-enabled applications.

Since the `IsoDdsKit::DdsEnabledAppDelegate` class provides some functionality
this delegate intends to augment, one augmented method must be marked with the
`override` keyword. Update the declaration of the `configureCmdLineArgs()`
method as follows:

```diff
-   void configureCmdLineArgs(CoreKit::Application *theApp);
+   virtual void configureCmdLineArgs(CoreKit::Application *theApp) override;
```

> [!NOTE]
> The additional decoration around the `configureCmdLineArgs()` method was not
> necessary in previous examples because the application's delegate class in
> those examples would derive directly from the `CoreKit::AppDelegate` class,
> which does not provide a definition or implementation for the method.

The `MyAppDelegate` class implements the `CoreKit::InterruptListener` interface
so that it can receive timer expiration events. Furthermore, the class has a
static utility method `EngineeringDataToString()` that is used to transform
`MyIsoSoftwareBus::EngineeringData` samples into human-readable strings.

Next, add the timer expiration event handler and instance fields to the
`MyAppDelegate` class in `src/MyAppDelegate.cpp` as follows:

```diff
    virtual void applicationDidTerminate(CoreKit::Application *theApp) override;
+
+   virtual void timerExpired(int timerId) override;
+
+   void onEngineeringData(MyIsoSoftwareBus::EngineeringData const& sample);
+
+private:
+   using EngrDataReaderContextPtr = std::shared_ptr< IsoDdsKit::ReaderContext< MyIsoSoftwareBus::EngineeringData > >;
+   using EngrDataWriterContextPtr = std::shared_ptr< IsoDdsKit::WriterContext< MyIsoSoftwareBus::EngineeringData > >;
+
+   std::uniform_int_distribution< int > m_distr;
+   EngrDataReaderContextPtr m_engrDataReaderContext;
+   EngrDataWriterContextPtr m_engrDataWriterContext;
+   std::random_device m_rng;
+   bool m_subscriberMode;
+   int m_timerId;
};
```

The `timerExpired()` event handler, along with the related `m_timerId` field,
should be familiar considering all the previous tutorial steps. The
`onEngineeringData()` event handler will be called whenever an application
instance in subscriber mode receives samples under the `EngineeringData` topic.
The `m_distr` and `m_rng` fields use productions from the C++ standard `random`
package to generate random numbers used to populate `EngineeringData` topic
samples. The `m_engrDataReaderContext` field, defined using the
`EngrDataReaderContextPtr` type alias, contains a smart pointer to an instance
of the `IsoDdsKit::ReaderContext<>` template class that integrates the DDS data
reader with the application's run loop. The `m_engrDataWriterContext` field,
defined using the `EngrDataWriterContextPtr` type alias, contains a smart
pointer to an instance of the `IsoDdsKit::WriterContext<>` template class that
integrates the DDS data writer with the application's run loop. The
`m_subscriberMode` is an internal flag indicating whether the application will
run as a subscriber (`true`) or a publisher (`false`).

### Implementation File Preamble

To set the stage for the implementation that will be added to the delegate
class, several files must be `#include`d, namespace imports identified, and
file-global constants defined. Add the following content near the beginning of
the `src/MyAppDelegate.cpp` class:

```diff
+#include <cstdlib>
+#include <functional>
+#include <sstream>
+#include <stdexcept>
+#include <vector>
+
#include "MyAppDelegate.h"

+using CoreKit::AppLog;
using CoreKit::Application;
+using CoreKit::EndLog;
+using CoreKit::G_MyApp;
+using IsoDdsKit::DdsEnabledAppDelegate;
+using MyIsoSoftwareBus::EngineeringData;
+
+namespace
+{
+   const char *SUBSCRIBER_FLAG = "subscriber";
+}
```

### Utility Function

The utility function that will be used to transform
`MySoftwareBus::EngineeringData` samples to string should appear next. Add the
following implementation for the `EngineeringDataToString()` static method:

```c++
std::string
MyAppDelegate::EngineeringDataToString(EngineeringData const& sample)
{
    std::stringstream result;

    result << "{ \"alpha\": " << sample.alpha()
        << ", \"bravo\": " << sample.bravo()
        << ", \"charlie\": " << sample.charlie()
        << " }";
    
    return result.str();
}
```

### Command Line Configuration

In this example application, the configuration that will be accepted via the
command line includes two (2) items:

* DDS domain identifier
* Application mode: publishing or subscribing

The `IsoDdsKit::DdsEnabledAppDelegate` class takes care of managing the DDS
domain identifier. To add support for the remaining input, first modify the
constructor implementation for the `MyAppDelegate` class in the
`src/MyAppDelegate.cpp` file as follows:

```diff
-MyAppDelegate::MyAppDelegate()
+MyAppDelegate::MyAppDelegate():
+   m_subscriberMode(false),
+   m_timerId(-1),
+   m_distr(0, 100)
{

}
```

The initialization of the `m_distr` field defines the range of possible values
the random number generator will produce when creating `EngineeringData` topic
samples: `[0, 100]`.

Next, add the following implementation for the `configureCmdLineArgs()` method
in the `src/MyAppDelegate.cpp` file:

```c++
void
MyAppDelegate::configureCmdLineArgs(Application *theApp)
{
    DdsEnabledAppDelegate::configureCmdLineArgs(theApp);

    theApp->addCmdLineArgDef(
        Application::CmdLineArg(
            SUBSCRIBER_FLAG,
            false,
            "If present, start the application in subscriber mode; start in publisher mode if absent."
        )
    );
}
```

This implementation uses the same presence/absence pattern for a command-line
flag as in [Step 6](./step_6.md#command-line-configuration) to indicate whether
the application should run in "subscriber" or "publisher" mode.

One novel aspect of this implementation is the use of the
`IsoDdsKit::DdsEnabledAppDelegate::configureCmdLineArgs()` base class method.
That implementation brings the logic that configures the command line
argument used to accept the DDS domain identifier.

### Application Initialization

The application initialization logic in this example will be responsible for
obtaining the application configuration from the command line and creating the
domain participant instance. Add the following implementation for the
`applicationDidInitialize()` method in the `src/MyAppDelegate.cpp` file:

```c++
void
MyAppDelegate::applicationDidInitialize(Application *theApp)
{
    DdsEnabledAppDelegate::applicationDidInitialize(theApp);
    m_subscriberMode = !(theApp->getCmdLineArgFor(SUBSCRIBER_FLAG).empty());

    // First call to participant() here creates DomainParticipant with default
    // QoS.
    topics().add< MyIsoSoftwareBus::EngineeringData >(participant(), "EngineeringData");
    if (m_subscriberMode)
    {
        theApp->log() << AppLog::LL_INFO << "Set application to subscriber mode." << EndLog;
        // First call to subscriber() here creates the Subscriber with default
        // QoS.
        auto dataReaderQos = subscriber().default_datareader_qos();
        dataReaderQos
            << dds::core::policy::Reliability::BestEffort()
            << dds::core::policy::History::KeepLast(100);
        m_engrDataReaderContext = createReaderContext< MyIsoSoftwareBus::EngineeringData >("EngineeringData", dataReaderQos);
    }
    else
    {
        theApp->log() << AppLog::LL_INFO << "Set application to publisher mode." << EndLog;
        // First call to publisher() here creates the Publisher with default
        // QoS.
        auto dataWriterQos = publisher().default_datawriter_qos();
        dataWriterQos
            << dds::core::policy::Reliability::BestEffort()
            << dds::core::policy::History::KeepLast(100);
        m_engrDataWriterContext = createWriterContext< MyIsoSoftwareBus::EngineeringData >("EngineeringData", dataWriterQos);
    }
    theApp->log() << AppLog::LL_INFO << "Opened domain participant to domain (" << participant().domain_id() << ")" << EndLog;
}
```

The `applicationDidInitialize()` implementation, like all examples up to this
point, accepts all values provided via the command line. The implementation
defers to its base class for some of that. In addition, the implementation
initializes the infrastructure used to interface with a DDS domain. As
indicated by the comment in the code, several of these infrastructure instances
are "lazily" created when called upon the first time. That is the case for the
`dds::domain::DomainParticipant`
(via `IsoDdsKit::DdsEnabledAppDelegate::participant()` method),
`dds::sub::Subscriber` (via `IsoDdsKit::DdsEnabledAppDelegate::subscriber()`
method), and `dds::pub::Publisher` (via
`IsoDdsKit::DdsEnabledAppDelegate::publisher()` method). The implementation
also uses the `IsoDdsKit::DdsEnabledAppDelegate::topics()` facility to create
the `EngineeringData` topic, specifying the topic's type via the method's
template argument.

If the application is configured in subscriber mode, the implementation
configures the quality of service settings for the DDS data reader then creates
the `IsoDdsKit::ReaderContext<>` instance, using the
`IsoDdsKit::DdsEnabledAppDelegate::createReaderContext()` method.

If the application is configured in publisher mode, the implementation
configures the quality of service settings for the DDS data writer then creates
the `IsoDdsKit::WriterContext<>` instance, using the
`IsoDdsKit::DdsEnabledAppDelegate::createWriterContext()` method.

### Application Start

Prior to turning control over to the main run loop, this application has to:

* For a subscriber role:
  * Register the `onEngineeringData()` method as an incoming sample event
    handler.
  * Activate the `ReaderContext<>` instance created in
    `applicationDidInitialize()`.
* For a publisher role:
  * Activate the `WriterContext<>` instance created in
    `applicationDidInitialize()`.
  * Set up a periodic 1Hz timer.

In the `src/MyAppDelegate.cpp` file, add the following implementation for the
`applicationStarting()` method:

```c++
void
MyAppDelegate::applicationStarting(Application *theApp)
{
    std::stringstream errorMsg;

    if (m_engrDataReaderContext)
    {
        // Subscriber mode prep
        m_engrDataReaderContext->sampleDistribution().addListener(std::bind(std::mem_fn(&MyAppDelegate::onEngineeringData), this, std::placeholders::_1));
        m_engrDataReaderContext->activate(theApp->getMainRunLoop());
    }
    else if (m_engrDataWriterContext)
    {
        // Publisher mode prep
        m_engrDataWriterContext->activate(theApp->getMainRunLoop());
        m_timerId = theApp->getMainRunLoop()->registerTimerWithInterval(1.0, this, true);
    }
    else
    {
        errorMsg << "Attempting to start without either publisher or subscriber.";
        theApp->log() << AppLog::LL_ERROR << errorMsg.str() << EndLog;
        throw std::logic_error(errorMsg.str());
    }
}
```

If the application instance was started as a subscriber, the method will
register the topic sample event handler using the
`IsoDdsKit::ReaderContext<>::sampleDistribution().addListener()` method
combination. Then, it activates the reader context using the
`IsoDdsKit::ReaderContext<>::activate()` method. The code relies on the
productions provided by the [standard C++ `functional` module][CxxFunctional]
via `std::bind()` and `std::mem_fn()`.

If, however, the application instance was started as a publisher, the method
activates the writer context using the `IsoDdsKit::WriterContext<>::activate()`
method.

### Application Pre-Termination

As the application winds down and prepares to exit out of the main run loop,
it must deactivate all of its stimuli sources. For this application, that
includes:

* Stop listening for `EngineeringData` samples (if in subscriber mode).
* Stop the periodic timer (if in publisher mode).
* Deactivate the appropriate context object.

Add the following implementation for the `applicationWillTerminate()` method
in the `src/MyAppDelegate.cpp` file:

```c++
void
MyAppDelegate::applicationWillTerminate(Application *theApp)
{
    if (m_engrDataReaderContext)
    {
        m_engrDataReaderContext->sampleDistribution().removeAllListeners();
        m_engrDataReaderContext->deactivate(theApp->getMainRunLoop());
    }

    if (m_engrDataWriterContext)
    {
        m_engrDataWriterContext->deactivate(theApp->getMainRunLoop());
    }

    if (m_timerId != -1)
    {
        theApp->getMainRunLoop()->deregisterTimer(m_timerId);
        m_timerId = -1;
    }
}
```

The method deactivates the appropriate context object, via the object's
`deactivate()` method. In addition:

* A subscriber application will deregister all event handlers (listeners)
  using the `sampleDistribution().removeAllListeners()` method combination.
* A publisher application will stop the periodic 1Hz timer.

### Application Post-Termination

Once the main run loop exits, all that is left to do is the releasing of all
resources. For this application that means tearing down the DDS infrastructure.
Add the following implementation to the `applicationDidTerminate()` method in
the `src/MyAppDelegate.cpp` file:

```c++
void
MyAppDelegate::applicationDidTerminate(Application *theApp)
{
    if (m_engrDataReaderContext)
    {
        m_engrDataReaderContext.reset();
    }

    if (m_engrDataWriterContext)
    {
        m_engrDataWriterContext.reset();
    }

    DdsEnabledAppDelegate::applicationDidTerminate(theApp);
}
```

The method relies on the delegate's base class implementation to completely
tear down the DDS infrastructure.

### Timer Expiration Handler

When the application is configured as a publisher, it emits an `EngineeringData`
sample every time the 1Hz timer configured in the
[Application Start](#application-start) expires. Add the following
implementation for the `timerExpired()` method in the `src/MyAppDelegate.cpp`
file:

```c++
void
MyAppDelegate::timerExpired(int timerId)
{
    EngineeringData sample;

    if (!m_engrDataWriterContext)
    {
        return;
    }

    sample.alpha(m_distr(m_rng));
    sample.bravo(m_distr(m_rng));
    sample.charlie(m_distr(m_rng));

    G_MyApp->log() << AppLog::LL_INFO << "Sending sample: " << EngineeringDataToString(sample) << EndLog;
    m_engrDataWriterContext->writer().write(sample);
}
```

After a sanity check to ensure that the writer context object is available,
this method creates a sample of type `MyIsoSoftwareBus::EngineeringData`, fills
it with random numbers using a combination of the `m_distr` and `m_rng` fields,
logs the impending action, and sends the sample using the writer context
object.

> [!NOTE]
> Why not fail with an exception at the pre-condition check?
>
> All of the pre-condition checks shown in examples up to this point include
> logic that fails with an exception if the condition is not met. All of those
> shown so far are present in the initialization stages of the application. The
> timer expiration handler, however, runs while the application main loop is
> active. The philosophy that Foundation developers have adopted to date is to
> fail loudly if something is wrong during application initialization, but
> favor continuous run loop operation as much as feasibly possible after it
> takes control. The pre-condition check could have been more sophisticated by
> emitting a warning message or even requesting an orderly application shutdown,
> but for this example it seems prudent to just silently ignore. The possibility
> of the situation arising is nearly non-existent anyways.

### Incoming Sample Handler

When the application is configured as a subscriber, it waits for incoming
samples and logs any that it observes. Add the following implementation for the
`onEngineeringData()` method in the `src/MyAppDelegate.cpp` class:

```c++
void
MyAppDelegate::onEngineeringData(EngineeringData const& sample)
{
    G_MyApp->log() << AppLog::LL_INFO << "Received sample: " << EngineeringDataToString(sample) << EndLog;
}
```

## Building Software Bus

Although the `MyIsoSoftwareBus` project is quite dissimilar to all the others
that have been configured up to this tutorial step, the build configuration
remains familiar. One unique pre-requisite, however, is determining the install
location for [Cyclone DDS][CycloneDDS]. The following example assumes that the
Iceoryx shared memory IPC is installed in `/opt/iceoryx/2.95.0` and
[Cyclone DDS][CycloneDDS] (both C and C++) is installed in
`/opt/cyclonedds/0.10.5`. From the root of the `MyIsoSoftwareBus` project tree,
execute the following commands:

```console
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCMAKE_PREFIX_PATH="/opt/iceoryx/2.95.0;/opt/cyclonedds/0.10.5" \
  -DWITH_DDS=CycloneDDS-CXX \
  -G Ninja \
  ..
```
Once the build environment is configured, the next step is to actually run the
build:

```console
$ ninja
```

## Building The Application

The procedure that builds the application is nearly identical to that of other
examples, with the exception that the build configuration must be told where to
find the `MyIsoSoftwareBus` package. That is done in a manner very similar to
how the build system is told where to find the Foundation SDK. From the root of
the `TutorialStep7a` project tree, execute the following commands:

```console
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCMAKE_PREFIX_PATH="/opt/iceoryx/2.95.0;/opt/cyclonedds/0.10.5" \
  -DFoundation_DIR=${FOUNDATION_ROOT}/build/Foundation \
  -DMyIsoSoftwareBus_DIR=$(readlink -f ../../MyIsoSoftwareBus/build/MyIsoSoftwareBus) \
  -G Ninja ..
```

The command, like the previous tutorial steps, assumes the `FOUNDATION_ROOT`
environment variable is set up. The build system is told where to find the
`MyIsoSoftwareBus` package via the `MyIsoSoftwareBus_DIR` variable. The command
line leverages the `readlink` shell tool to transform the relative path to the
`MyIsoSoftwareBus` build tree to an absolute path.

> [!NOTE]
> Notice the pattern between the `Foundation_DIR` and `MyIsoSoftwareBus_DIR`
> variables? In CMake, the `find_package()` method will attempt to find the
> configuration file for the requested package in a set of well-known locations.
> When that fails (and for those custom packages, that initial search *will*
> fail), CMake tries to look in a directory pointed to by a variable following
> the name pattern `<package>_DIR` where `<package>` is the package name. Thus,
> for a CMake command that reads `find_package(Foundation ...)`, it will check
> for the presence of a `Foundation_DIR` variable. The same goes for the
> `MyIsoSoftwareBus` package.

After the build environment is configured, all that is left is to actually
build:

```console
$ ninja
```

## Running

### Running The Subscriber

From a terminal shell session with a current working directory in the
`TutorialStep7a` build tree, execute the application
as follows:

```console
$ ./IsoSoftwareBusPrimer --dds-domain=30 --subscriber
```

At startup, the application will at most indicate that the DDS system has been
activated with a `Started DDS Participant` log message, but will not report any
other activity.

### Running The Publisher

From a terminal shell session with a current working directory in the
`TutorialStep7a` build tree, execute the application as follows:

```console
$ ./IsoSoftwareBusPrimer --dds-domain=30
```

The application should start after a short while and begin reporting on the
samples it sends:

```
[2024-09-16T17:37:53.909Z] [IsoSoftwareBusPrimer] [1574026] [INFORMATION]: Set application to publisher mode.
[2024-09-16T17:37:53.909Z] [IsoSoftwareBusPrimer] [1574026] [INFORMATION]: Opened domain participant to domain (30)
[2024-09-16T17:37:54.910Z] [IsoSoftwareBusPrimer] [1574026] [INFORMATION]: Sending sample: { "alpha": 69, "bravo": 54, "charlie": 39 }
[2024-09-16T17:37:55.910Z] [IsoSoftwareBusPrimer] [1574026] [INFORMATION]: Sending sample: { "alpha": 41, "bravo": 41, "charlie": 12 }
[2024-09-16T17:37:56.910Z] [IsoSoftwareBusPrimer] [1574026] [INFORMATION]: Sending sample: { "alpha": 33, "bravo": 60, "charlie": 41 }
[2024-09-16T17:37:57.910Z] [IsoSoftwareBusPrimer] [1574026] [INFORMATION]: Sending sample: { "alpha": 37, "bravo": 86, "charlie": 59 }
[2024-09-16T17:37:58.910Z] [IsoSoftwareBusPrimer] [1574026] [INFORMATION]: Sending sample: { "alpha": 98, "bravo": 82, "charlie": 15 }
[2024-09-16T17:37:59.910Z] [IsoSoftwareBusPrimer] [1574026] [INFORMATION]: Sending sample: { "alpha": 70, "bravo": 46, "charlie": 42 }
```

Back in the terminal window where the subscribing application was started, more
log message activity evinces receipt of samples transmitted by the publishing
application:

```
[2024-09-16T17:37:17.527Z] [IsoSoftwareBusPrimer] [1574008] [INFORMATION]: Set application to subscriber mode.
[2024-09-16T17:37:17.529Z] [IsoSoftwareBusPrimer] [1574008] [INFORMATION]: Opened domain participant to domain (30)
[2024-09-16T17:37:54.911Z] [IsoSoftwareBusPrimer] [1574008] [INFORMATION]: Received sample: { "alpha": 69, "bravo": 54, "charlie": 39 }
[2024-09-16T17:37:55.911Z] [IsoSoftwareBusPrimer] [1574008] [INFORMATION]: Received sample: { "alpha": 41, "bravo": 41, "charlie": 12 }
[2024-09-16T17:37:56.910Z] [IsoSoftwareBusPrimer] [1574008] [INFORMATION]: Received sample: { "alpha": 33, "bravo": 60, "charlie": 41 }
[2024-09-16T17:37:57.911Z] [IsoSoftwareBusPrimer] [1574008] [INFORMATION]: Received sample: { "alpha": 37, "bravo": 86, "charlie": 59 }
[2024-09-16T17:37:58.911Z] [IsoSoftwareBusPrimer] [1574008] [INFORMATION]: Received sample: { "alpha": 98, "bravo": 82, "charlie": 15 }
[2024-09-16T17:37:59.911Z] [IsoSoftwareBusPrimer] [1574008] [INFORMATION]: Received sample: { "alpha": 70, "bravo": 46, "charlie": 42 }
```

## Conclusion

Congratulations! 🎉🎉 You have created a Foundation application that communicates
over a DDS-based software bus with other software components. Although the
actual application design is quite simple, the discussion for this example is
quite long as it introduces several new concepts. The next tutorial steps will
build upon this success to eventually create a small system of software
components that can emulate command and control of a device.


[CxxFunctional]: https://en.cppreference.com/w/cpp/utility/functional "C++ Functional Module"
[CycloneDDS]: https://cyclonedds.io/ "Cyclone DDS"
[OpenSplice]: https://github.com/ADLINK-IST/opensplice "Vortex OpenSplice Community Edition"
[omg_dds_spec]: https://www.omg.org/spec/DDS/ "OMG DDS Specification"
[omg_dds_xml_spec]: https://www.omg.org/spec/DDS-XML/ "OMG DDS XML Syntax Specification"
[omg_idl_spec]: https://www.omg.org/spec/IDL/ "OMG IDL Specification"
[xpath_intro]: https://www.w3schools.com/xml/xpath_intro.asp "W3Schools XPath Introduction"
