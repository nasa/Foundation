# Tutorial Step 7: Software Bus Participation Basics (Classic Variant)

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
[OpenSplice] DDS provider, and the distribution as built happens to include one
in its `<version>/HDE/<platform>/docs/html/DDSTutorial` folder, where
`<version>` is the version tag for the distribution (`6.9.210323OSS` as of this
writing), and `<platform>` is the platform label assigned to the build.

## The Software Bus

In Foundation, the sample and topic definitions are collectively referred to as
the "Software Bus." Although it would be possible to establish integration of
multiple application by just sharing the message structure definitions written
in IDL, the approach this example will adopt groups both the message and topic
definitions into a single library that applications can link to and leverage.

> [!NOTE]
> Although in modern DDS distributions it is possible to group all of the
> aforementioned Software Bus aspects into specifications
> [written in XML][omg_dds_xml_spec], at the time the Foundation library was
> conceived neither those specifications, nor implementations based on them,
> were available.

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
located, give the project folder the name `MySoftwareBus` and copy over the
Foundation-provided software bus skeleton with the following commands (the
`FOUNDATION_ROOT` environment variable points to the Foundation root folder):

```console
$ mkdir MySoftwareBus
$ cd MySoftwareBus
$ rsync -auv ${FOUNDATION_ROOT}/examples/SampleSoftwareBus/ .
```

Once copied over, the structure of the project tree should be as follows:

```
MySoftwareBus
├─cmake
│ ├─Modules
│ │ ├─CoreDXTargetIDLSources.cmake
│ │ ├─FindCoreDX.cmake
│ │ ├─FindOpenSplice.cmake
│ │ ├─OpenSpliceTargetIDLSources.cmake
│ │ └─TargetIDLSources.cmake
│ └─SampleSoftwareBusConfig.cmake
├─CMakeLists.txt
└─src
  └─ProviderName
    ├─SampleTopicConfig.cpp.template
    └─SampleTopicConfig.h.template
```

Each file and folder serves a different purpose. Several of them are identical
to those included in the sample application skeleton. Refer to
[Step 1](./step_1.md#bootstrap-the-project-tree) for descriptions on files not
explained here:

* `cmake/SampleSoftwareBusConfig.cmake` - Contains package configuration entry
  point meant for use by applications that link against this library.
* `src/ProviderName` - Folder that will contain the topic configuration source
  files. Must be renamed to bear the selected DDS provider.
* `src/ProviderName/SampleTopicConfig.cpp.template` - C++ source file template
  for topic configuration definitions.
* `src/ProviderName/SampleTopicConfig.h.template` - C++ header file template for
  topic configuration definitions.

This example will base its software bus on the [OpenSplice] DDS provider, since
it is the most readily available. Rename the `src/ProviderName` directory to be
`src/OpenSplice` from the project root folder:

```console
$ mv src/ProviderName src/OpenSplice
```

> [!NOTE]
> The Foundation build tree that this example will use must have also been built
> with [OpenSplice] as its DDS provider. Refer to the
> [Foundation build](../building.md) page for more information on how to select
> [OpenSplice] as the DDS provider for Foundation.

## Application Project Bootstrap

Since the application will be significantly different from those created in
previous tutorial steps, it will start from the baseline application skeleton
provided by the Foundation. In a terminal shell session with the current
directory being the one containing the previously-created `MySoftwareBus`
directory, give the application project folder the name `TutorialStep7` and
copy over the baseline skeleton with the following commands (the
`FOUNDATION_ROOT` environment variable points to the Foundation root folder):

```console
$ mkdir TutorialStep7
$ cd TutorialStep7
$ rsync -auv ${FOUNDATION_ROOT}/examples/SampleAppSkeleton/ .
```

## Software Bus Library Build Configuration

Build configuration involves a bit more for the software bus project than what
has been done in previous tutorial steps. The primary reason for the difference
is the fact that the project must produce both a library that applications can
link against, as well as the required configuration that the applications will
use to configure themselves for linking/importing the library in. The
application project being built in this tutorial step will leverage this
configuration to link in the software bus library. First, in the `MySoftwareBus`
project directory, modify the project metadata towards the beginning of the
`CMakeLists.txt` file as follows:

```diff
cmake_minimum_required(VERSION 3.16)

-project(SampleSoftwareBus LANGUAGES CXX)
+project(MySoftwareBus VERSION 1.0.0 LANGUAGES CXX)
```

Next, modify the set of source files used to build the application by updating
the `add_library()` command as follows:

```diff
add_library(${PROJECT_NAME} STATIC
-   "<insert topic configuration C++ files to build>"
+   "src/${WITH_DDS}/CommandConfig.cpp"
+   "src/${WITH_DDS}/EngineeringDataConfig.cpp"
)
```

After that, modify the `target_idl_sources()` command, which adds C++ files
generated from IDL to the library target, as follows:

```diff
target_idl_sources(${PROJECT_NAME}
    DDS_PROVIDER ${WITH_DDS}
-   IDL_SOURCES "<insert IDL file path>"
+   IDL_SOURCES "src/MySoftwareBus.idl"
+   BINDING CXX
)
```

Finally (at least as the `CMakeLists.txt` file is concerned), modify the
`install()` command that deploys/installs the library header files so it
includes the headers that define the topic configuration:

```diff
install(
    FILES
-       "<insert topic configuration header files>"
+       "src/${WITH_DDS}/CommandConfig.h"
+       "src/${WITH_DDS}/EngineeringDataConfig.h"
        ${IDL_LANG_BINDING_HEADERS}
    DESTINATION include
    Component Devel
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
support for DDS. In the `TutorialStep7` directory, edit the project metadata in
the `CMakeLists.txt` `project()` command as follows:

```diff
- project(SampleApp LANGUAGES CXX)
+ project(SoftwareBusPrimer LANGUAGES CXX)
```

Then, update the project dependencies in the `find_package()` command:

```diff
-find_package(Foundation REQUIRED CoreKit)
+find_package(Foundation REQUIRED COMPONENTS CoreKit DdsKit)
+find_package(MySoftwareBus REQUIRED)
```

The `DdsKit` name identifies the module that brings in the Foundation DDS
functionality the application will use. In addition, there's a new
`find_package()` statement that imports the software bus library being created
in this example step. The application build configuration command shows how the
`cmake` tool will find the `MySoftwareBus` package.

Finally, include the required components in the `target_link_libraries()`
command as follows:

```diff
-target_link_libraries(${PROJECT_NAME} Foundation::CoreKit)
+target_link_libraries(${PROJECT_NAME} Foundation::CoreKit Foundation::DdsKit MySoftwareBus::MySoftwareBus)
```

The `target_link_libraries()` command statement folds in the `DdsKit` and
`MySoftwareBus` components, identified by their fully-scoped names.

## Software Bus Topic Configuration

As initially enumerated in the [example design](#example-design), the software
bus library will define two (2) message structures. One structure will be
designed to carry engineering data for a fictional instrument. The other
structure will be designed to carry commands as dispatched by an operator. Add
the following content to the `src/MySoftwareBus.idl` IDL source file:

```
module MySoftwareBus
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
> The `#pragma keylist` pre-processor definitions are required by [OpenSplice]
> so that it knows the mentioned type name will be used as a topic sample type.
> Other DDS providers may have different requirements regarding the
> identification of types for topic samples.

This example will only make use of the `EngineeringData` message type. The
`Command` message type will be saved for use in future examples.

### EngineeringData Topic Configuration

Next, each topic needs a data structure definition that contains enough
information about the topic for the Foundation `DdsKit` to use it. The project
skeleton/template imported in the
[bootstrap step](#software-bus-library-bootstrap) brought with it two (2)
template files that lay out the contents for this structure definition. Although
we could detail instructions on how to copy and modify the template to suit the
library's needs, it is probably quicker to just lay out the contents. Create
the topic configuration header file `src/OpenSplice/EngineeringDataConfig.h`
with the following content:

```c++
#ifndef _MY_SOFTWARE_BUS_SRC_OPENSPLICE_ENGINEERING_DATA_CONFIG_H_
#define _MY_SOFTWARE_BUS_SRC_OPENSPLICE_ENGINEERING_DATA_CONFIG_H_

#include <MySoftwareBusSplDcps.h>

struct EngineeringDataConfig
{
    using SampleType = MySoftwareBus::EngineeringData;
    using SampleSeqType = MySoftwareBus::EngineeringDataSeq;
    using TypeSupportType = MySoftwareBus::EngineeringDataTypeSupport;
    using ReaderPtrType = MySoftwareBus::EngineeringDataDataReader_ptr;
    using ReaderType = MySoftwareBus::EngineeringDataDataReader;
    using WriterPtrType = MySoftwareBus::EngineeringDataDataWriter_ptr;
    using WriterType = MySoftwareBus::EngineeringDataDataWriter;
    static const char *TopicName;
    static const bool NormallyReliable;
    static const unsigned DefaultHistoryDepth;
    static const char *TypeNameAlias;
    static const unsigned MaxSamplesPerSecond;
};

#endif /* !_MY_SOFTWARE_BUS_SRC_OPENSPLICE_ENGINEERING_DATA_CONFIG_H_ */

// vim: set ts=4 sw=4 expandtab:
```

The data structure is not meant to be instantiated, but rather to serve as an
information source as-is. `DdsKit` requires that the data structure provide the
following type aliases and static fields:

* `SampleType` - Type alias to the C++ data type emitted by the DDS provider for
  the topic sample type as defined in the IDL. For the `EngineeringData` topic
  as defined in the IDL, [OpenSplice] emits a C++ type that follows the pattern
  `<module name>::<struct name>`.
* `SampleSeqType` - Type alias to the C++ data type created by the DDS provider
  as a convenience that holds a sequence (i.e., a variable-size list) of sample
  values. In [OpenSplice], the emitted C++ type follows the pattern
  `<module name>::<struct name>Seq`.
* `TypeSupportType` - Type alias to the C++ class created by the DDS provider to
  register its use with the DDS domain participant. In [OpenSplice], the emitted
  C++ class name follows the pattern `<module name>::<struct name>TypeSupport`.
* `ReaderPtrType` - Type alias to the C++ pointer type for the data reader class
  of the sample type as emitted by the DDS provider. [OpenSplice] defines a
  smart pointer class that must be used instead of traditional C/C++ pointers.
  The class follows the pattern `<module name>::<struct name>DataReader_ptr`.
* `ReaderType` - Type alias to the C++ data reader class for the sample as
  emitted by the DDS provider. In [OpenSplice], the data reader class name
  follows the pattern `<module name>::<struct name>DataReader`.
* `WriterPtrType` - Type alias to the C++ pointer type for the data writer class
  of the sample type as emitted by the DDS provider. [OpenSplice] defines a
  smart pointer class that must be used instead of traditional C/C++ pointers.
  The class follows the pattern `<module name>::<struct name>DataWriter_ptr`.
* `WriterType` - Type alias to the C++ data writer class for the sample as
  emitted by the DDS provider. In [OpenSplice], the data writer class name
  follows the pattern `<module name>::<struct name>DataWriter`.
* `TopicName` - Name to use when creating the topic. Convention would have this
  be the text preceding `Config` in the `EngineeringDataConfig` structure name.
* `NormallyReliable` - `true` if the topic should be configured with reliable
  transport quality-of-service; `false` otherwise.
* `DefaultHistoryDepth` - Unsigned integer that defines the sample history
  depth for the topic.
* `TypeNameAlias` - Text identifier for the data type used to transact topic
  samples. Typically should be the fully-scoped name of the emitted C++ sample
  type.
* `MaxSamplesPerSecond` - Currently unused; reserved for future expansion.

Next, create the topic configuration implementation file
`src/OpenSplice/EngineeringDataConfig.cpp` with the following content:

```c++
#include <limits>

#include "EngineeringDataConfig.h"


const char *EngineeringDataConfig::TopicName = "EngineeringData";
const bool EngineeringDataConfig::NormallyReliable = false;
const unsigned EngineeringDataConfig::DefaultHistoryDepth = 100u;
const char *EngineeringDataConfig::TypeNameAlias = "MySoftwareBus::EngineeringData";
const unsigned EngineeringDataConfig::MaxSamplesPerSecond = std::numeric_limits< unsigned >::max();

// vim: set ts=4 sw=4 expandtab:
```

The meaning of the fields in the data structure have already been discussed. For
this particular topic, the choices made that should be noted follow:

* The name of the topic mimics that of the data type for its samples:
  `EngineeringData`.
* Since the topic publisher is expected to emit samples at a regular interval,
  missing one here and there is not a big deal, so the transport
  quality-of-service is not set to be reliable.
* The size of the sample backlog afforded to applications reading samples is set
  to `100`.

> [!NOTE]
> Although the name of the topic and data type in this example are identical,
> that need not be the case with DDS. The topic name could be completely
> different between them, and as long as the publishers and subscribers match
> the pair of names, everything should work fine.

> [!NOTE]
> Wny not use reliable transport?
>
> It may seem a bit cavalier to say that sample loss is "no big deal" and opting
> to not use reliable messaging for the topic, but the making of that decision
> considers two facts: that `EngineeringData` samples are updated regularly, and
> that reliable messaging incurs additional costs to both processing power and
> memory footprint. In this case, the balance swayed towards the "acceptable
> losses" approach. In other cases, the processor/memory expense may be worth
> mitigating the possible losses.

### Command Topic Configuration

Although it will not be used in this example, it is still prudent to offer topic
configurations for all available in the `MySoftwareBus` package. The structure
of the `Command` topic configuration is identical to that of
[`EngineeringData`](#engineeringdata-topic-configuration). Add the file
`src/OpenSplice/CommandConfig.h` to the `MySoftwareBus` project with the
following content:

```c++
#ifndef _MY_SOFTWARE_BUS_SRC_OPENSPLICE_COMMAND_CONFIG_H_
#define _MY_SOFTWARE_BUS_SRC_OPENSPLICE_COMMAND_CONFIG_H_

#include <MySoftwareBusSplDcps.h>

struct CommandConfig
{
    using SampleType = MySoftwareBus::Command;
    using SampleSeqType = MySoftwareBus::CommandSeq;
    using TypeSupportType = MySoftwareBus::CommandTypeSupport;
    using ReaderPtrType = MySoftwareBus::CommandDataReader_ptr;
    using ReaderType = MySoftwareBus::CommandDataReader;
    using WriterPtrType = MySoftwareBus::CommandDataWriter_ptr;
    using WriterType = MySoftwareBus::CommandDataWriter;
    static const char *TopicName;
    static const bool NormallyReliable;
    static const unsigned DefaultHistoryDepth;
    static const char *TypeNameAlias;
    static const unsigned MaxSamplesPerSecond;
};

#endif /* !_MY_SOFTWARE_BUS_SRC_OPENSPLICE_COMMAND_CONFIG_H_ */

// vim: set ts=4 sw=4 expandtab:
```

Next, add the `src/OpenSplice/CommandConfig.cpp` file with the following
content:

```c++
#include <limits>

#include "CommandConfig.h"


const char *CommandConfig::TopicName = "Command";
const bool CommandConfig::NormallyReliable = true;
const unsigned CommandConfig::DefaultHistoryDepth = 10u;
const char *CommandConfig::TypeNameAlias = "MySoftwareBus::Command";
const unsigned CommandConfig::MaxSamplesPerSecond = std::numeric_limits< unsigned >::max();

// vim: set ts=4 sw=4 expandtab:
```

There are a few notable differences that should be pointed out:

* Given that commands are irregular, important, and sometimes irreplaceable, the
  expense of using reliable messaging, as evinced by the value assigned to the
  `NormallyReliable` field, is warranted.
* Also given their irregularity, the history depth can be shallower than that of
  `EngineeringData`.

## Application Main Function

There are very little changes required of the main function, just like in
[Step 1](./step_1.md). The change involves altering the application name for
logging purposes in the call to `::CoreKit::Application::initialize()`. From the
`TutorialStep7` project folder modify the `src/main.cpp` file as follows:

```diff
- G_MyApp->initialize("ApplicationName", argc, argv);
+ G_MyApp->initialize("SoftwareBusPrimer", argc, argv);
```

## Application Delegate

### Class Structure Customization

Since the logic in this example is relatively simple, it will all be contained
within the application delegate class. From the `TutorialStep7` project folder,
modify the beginning of the `src/MyAppDelegate.h` file as follows:

```diff
#ifndef _MY_APP_DELEGATE_H_
#define _MY_APP_DELEGATE_H_

+#include <memory>
+#include <string>
#include <CoreKit/CoreKit.h>
+#include <DdsKit/DdsKit.h>
+#include <EngineeringDataConfig.h>

-class MyAppDelegate : public CoreKit::AppDelegate
+class MyAppDelegate : public CoreKit::AppDelegate, public CoreKit::InterruptListener
{
public:
+   static std::string EngineeringDataToString(MySoftwareBus::EngineeringData const* sample);
+
    MyAppDelegate();
```

The `MyAppDelegate` class implements the `CoreKit::InterruptListener` interface
so that it can receive timer expiration events. Furthermore, the class has a
static utility method `EngineeringDataToString()` that is used to transform
`MySoftwareBus::EngineeringData` samples into human-readable strings.

Next, add the timer expiration event handler and instance fields to the
`MyAppDelegate` class in `src/MyAppDelegate.cpp` as follows:

```diff
    virtual void applicationDidTerminate(CoreKit::Application *theApp) override;
+
+   virtual void timerExpired(int timerId) override;
+
+   void onEngineeringData(MySoftwareBus::EngineeringData *sample);
+
+private:
+   int m_ddsDomainId;
+   DdsKit::ConfiguredDdsTopic< EngineeringDataConfig > *m_engrDataTopic;
+   std::unique_ptr< DdsKit::BasicDdsParticipant > m_participant;
+   bool m_subscriberMode;
+   int m_timerId;
};
```

The `timerExpired()` event handler should be familiar considering all the
previous tutorial steps. The `onEngineeringData()` event handler will be called
whenever an application instance in subscriber mode receives samples under the
`EngineeringData` topic. The `m_ddsDomainId` field contains the identifier for
the DDS domain the application will participate in. The `m_participant` field
holds the instance to the `DdsKit::BasicDdsParticipant` used by the application.
The `m_subscriberMode` is an internal flag indicating whether the application
will run as a subscriber (`true`) or a publisher (`false`).

The `m_engrDataTopic` is a reference to the `DdsKit::ConfiguredDdsTopic<>`
instance that represents the `EngineeringData` topic. The utility of the
`EngineeringDataConfig` data structure defined in the `MySoftwareBus` package
is evident now: the `DdsKit::ConfiguredDdsTopic<>` template class uses the type
and static constant definitions in the `EngineeringDataConfig` structure type
provided as the sole template argument to "complete" its implementation.

> [!NOTE]
> Why the use of bare C++ pointer for the `m_engrDataTopic` field?
>
> Throughout all of the examples the use of modern C++ smart pointers via
> `std::unique_ptr` has been plentiful. Thus, it seems odd to use a so-called
> "bare, bald" C++ pointer for the instance field representing the
> `EngineeringData` topic. This is so because of two (2) reasons. First, the
> topic object instance is not owned by the application, but rather by the
> `DdsKit::BasicDdsParticipant` instance; it remains in the application merely
> as a "weak reference." Second, based on the previous statement, modern C++
> would have us use the `std::shared_ptr`/`std::weak_ptr` combo for such a
> situation, but neither was widely available when Foundation was first
> conceived.

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
#include "MyAppDelegate.h"

+using CoreKit::AppLog;
using CoreKit::Application;
+using CoreKit::EndLog;
+using CoreKit::G_MyApp;
+using DdsKit::BasicDdsParticipant;
+using DdsKit::ConfiguredDdsTopic;
+using DdsKit::newCallbackForTopic;
+using MySoftwareBus::EngineeringData;
+
+namespace
+{
+   const char *DOMAIN_ID_FLAG = "dds-domain";
+   const char *SUBSCRIBER_FLAG = "subscriber";
+}
```

### Utility Function

The utility function that will be used to transform
`MySoftwareBus::EngineeringData` samples to string should appear next. Add the
following implementation for the `EngineeringDataToString()` static method:

```c++
std::string
MyAppDelegate::EngineeringDataToString(EngineeringData const* sample)
{
    std::stringstream result;

    result << "{ \"alpha\": " << sample->alpha
        << ", \"bravo\": " << sample->bravo
        << ", \"charlie\": " << sample->charlie
        << " }";
    
    return result.str();
}
```

### Command Line Configuration

In this example application, the configuration that will be accepted via the
command line includes two (2) items:

* DDS domain identifier
* Application mode: publishing or subscribing

To add support for this input, first modify the constructor implementation for
the `MyAppDelegate` class in the `src/MyAppDelegate.cpp` file as follows:

```diff
-MyAppDelegate::MyAppDelegate()
+MyAppDelegate::MyAppDelegate():
+   m_ddsDomainId(-1),
+   m_engrDataTopic(nullptr),
+   m_subscriberMode(false),
+   m_timerId(-1)
{

}
```

Next, add the following implementation for the `configureCmdLineArgs()` method
in the `src/MyAppDelegate.cpp` file:

```c++
void
MyAppDelegate::configureCmdLineArgs(Application *theApp)
{
    theApp->addCmdLineArgDef(
        Application::CmdLineArg(
            DOMAIN_ID_FLAG,
            true,
            "Identifier of the DDS domain to participate in."
        )
    );
    theApp->addCmdLineArgDef(
        Application::CmdLineArg(
            SUBSCRIBER_FLAG,
            false,
            "If present, start the application in subscriber mode; start in publisher mode if absent."
        )
    );
    BasicDdsParticipant::AddDdsRelatedCmdArgs(theApp);
}
```

This implementation uses the same presence/absence pattern for a command-line
flag as in [Step 6](./step_6.md#command-line-configuration) to indicate whether
the application should run in "subscriber" or "publisher" mode.

One novel aspect of this implementation is the use of the
`DdsKit::BasicDdsParticipant::AddDdsRelatedCmdArgs()` static method. Depending
on the DDS provider selected when the Foundation SDK was built, this method will
add a set of command-line arguments specific to that DDS provider.

### Application Initialization

The application initialization logic in this example will be responsible for
obtaining the application configuration from the command line and creating the
domain participant instance. Add the following implementation for the
`applicationDidInitialize()` method in the `src/MyAppDelegate.cpp` file:

```c++
void
MyAppDelegate::applicationDidInitialize(Application *theApp)
{
    std::vector< std::string > ddsProviderArgs;
    std::stringstream participantName;

    m_ddsDomainId = std::stoi(theApp->getCmdLineArgFor(DOMAIN_ID_FLAG));
    m_subscriberMode = !(theApp->getCmdLineArgFor(SUBSCRIBER_FLAG).empty());
    ddsProviderArgs = BasicDdsParticipant::GatherDdsRelatedCmdArgs(theApp);

    participantName << theApp->log().appName() << "[" << (m_subscriberMode ? "Subscriber" : "Publisher") << "]";
    m_participant.reset(
        new BasicDdsParticipant(
            m_ddsDomainId,
            participantName.str(),
            ddsProviderArgs,
            &theApp->log()
        )
    );
}
```

The `applicationDidInitialize()` implementation is very similar to that in other
example applications, except for a few things. The method logic extracts any
DDS-provider related arguments from the command line using the
`DdsKit::BasicDdsParticipant::GatherDdsRelatedCmdArgs()` static method and
stores them into the `participantName` collection. This collection is then
passed to the `DdsKit::BasicDdsParticipant` constructor for further processing.

### Application Start

Prior to turning control over to the main run loop, this application has to set
up the `EngineeringData` DDS topic and, depending on its assigned role, prepare
other stimuli sources:

* Activate the DDS system.
* Create the `EngineeringData` topic object.
* For a subscriber role:
  * Set up the `EngineeringData` topic to accept incoming samples.
  * Register the `onEngineeringData()` method as an incoming sample event
    handler.
* For a publisher role:
  * Set up the `EngineeringData` topic to emit samples.
  * Set up a periodic 1Hz timer.

In the `src/MyAppDelegate.cpp` file, add the following implementation for the
`applicationStarting()` method:

```c++
void
MyAppDelegate::applicationStarting(Application *theApp)
{
    if (!m_participant)
    {
        throw std::runtime_error("Attempting to start without DDS domain participant.");
    }
    m_participant->startDds();
    // Although the topic object is created here, it is "adopted" and
    // subsequently managed by the BasicDdsParticipant instance.
    m_engrDataTopic = new ConfiguredDdsTopic< EngineeringDataConfig >(
        theApp->getMainRunLoop(),
        m_participant.get()
    );

    if (m_subscriberMode)
    {
        m_engrDataTopic->createReader(EngineeringDataConfig::NormallyReliable);
        m_engrDataTopic->registerCallback(
            newCallbackForTopic(
                EngineeringDataConfig(),
                std::bind(
                    std::mem_fn(&MyAppDelegate::onEngineeringData),
                    this,
                    std::placeholders::_1
                )
            )
        );
        m_engrDataTopic->listenForSamples();
    }
    else
    {
        m_engrDataTopic->createWriter(EngineeringDataConfig::NormallyReliable);
        m_timerId = theApp->getMainRunLoop()->registerTimerWithInterval(1.0, this, true);
    }
}
```

The `startDds()` method in `DdsKit::BasicDdsParticipant` activates the DDS
communications and *must* be called prior to creating any topic objects. The
`EngineeringData` topic instance is constructed in this method, but it is
immediately adopted by the `DdsKit::BasicDdsParticipant` instance, leaving us
with the weak reference referred to in the
[Class Structure Customization section](#class-structure-customization).

If the application instance was started as a subscriber, the method will
configure the topic object to accept samples by first having it set up a reader
with the `createReader()` method, registering an incoming sample event handler
with the `registerCallback()` method and the `newCallbackForTopic()` factory
function, and starting the sample listening logic with the `listenForSamples()`
method.

> [!NOTE]
> It may see odd that the call to the factory function `newCallbackForTopic()`
> accepts an instance of the topic configuration data structure, given that the
> tutorial instructions already stated that the data structure is not meant to
> be instantiated. The instance offered as the first argument, however, is only
> used to allow the factory function, which happens to be a template function,
> to infer the topic configuration structure type. Otherwise, the call syntax
> for the factory function would have been a little more complicated, as it
> would have required explicit statement of *all* template arguments.

If, however, the application instance was started as a publisher, the method
will configure the topic object to emit samples by first having it set up a
writer with the `createWriter()` method. Then, the method will start a periodic
1Hz timer with the main run loop, registering the application delegate itself
as the event handler.

> [!NOTE]
> The options of either receiving or emitting samples using a topic object may
> seem mutually-exclusive, but in reality they are not. It is possible to
> configure a `DdsKit::ConfiguredDdsTopic` instance to both emit and accept
> samples, but this would require implementing a form of filtering that prevents
> self-publishing feedback. Discussion on such filtering techniques, either
> intrinsic to DDS or otherwise, is beyond the scope of this tutorial.

### Application Pre-Termination

As the application winds down and prepares to exit out of the main run loop,
it must deactivate all of its stimuli sources. For this application, that
includes:

* Stop listening for `EngineeringData` samples (if in subscriber mode).
* Stop the periodic timer (if in publisher mode).
* Deactivate the DDS system.

Add the following implementation for the `applicationWillTerminate()` method
in the `src/MyAppDelegate.cpp` file:

```c++
void
MyAppDelegate::applicationWillTerminate(Application *theApp)
{
    if ((nullptr == m_engrDataTopic) || !m_participant)
    {
        return;
    }

    if (m_subscriberMode)
    {
        m_engrDataTopic->stopListening();
    }
    else
    {
        theApp->getMainRunLoop()->deregisterTimer(m_timerId);
        m_timerId = -1;
    }

    m_participant->stopDds();
    m_engrDataTopic = nullptr;
}
```

The method, after making some cursory pre-condition checks, tears down the
stimuli sources differently, depending on the application instance's assigned
role:

* A subscriber application will stop listening for incoming `EngineeringData`
  samples
* A publisher application will stop the periodic 1Hz timer.

After that, the method deactivates the DDS communications. Once that's done, the
weak reference to the `ConfiguredDdsTopic` instance in `m_engrDataTopic` becomes
invalid, and thus it is "nulled out" accordingly.

### Application Post-Termination

Once the main run loop exits, all that is left to do is the releasing of all
resources. For this application that simply means de-allocating the
`BasicDdsParticipant` instance. Add the following implementation to the
`applicationDidTerminate()` method in the `src/MyAppDelegate.cpp` file:

```c++
void
MyAppDelegate::applicationDidTerminate(Application *theApp)
{
    m_participant.reset(nullptr);
}
```

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
    EngineeringData aSample;

    if (nullptr == m_engrDataTopic)
    {
        return;
    }

    aSample.alpha = random() % 100;
    aSample.bravo = random() % 100;
    aSample.charlie = random() % 100;

    G_MyApp->log() << AppLog::LL_INFO << "Sending sample: " << EngineeringDataToString(&aSample) << EndLog;
    m_engrDataTopic->writeSample(&aSample);
}
```

After a sanity check to ensure that the topic object is available, this method
creates a sample of type `MySoftwareBus::EngineeringData`, fills it with random
numbers in the range `[0, 100)`, logs the impending action, and sends the sample
using the topic object instance.

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
MyAppDelegate::onEngineeringData(EngineeringData *sample)
{
    G_MyApp->log() << AppLog::LL_INFO << "Received sample: " << EngineeringDataToString(sample) << EndLog;
}
```

## Building Software Bus

Although the `MySoftwareBus` project is quite dissimilar to all the others that
have been configured up to this tutorial step, the build configuration remains
familiar. One unique pre-requisite, however, is that the shell environment where
the build (and application) will run must be prepared prior. [OpenSplice] brings
such an environment setup script located in the `<version>/HDE/<platform>/`
folder of the package's installation root called `release.com`. When run under
a Bourne-Again Shell (`bash`), it can be simply sourced into the current
shell environment. In the following example, the root installation folder for
[OpenSplice] is `/opt/VortexOpenSplice`, the version is `6.9.210323OSS`, and the
platform is `x86_64.linux_clang-dev`:

```console
$ . /opt/VortexOpenSplice/6.9.210323OSS/HDE/x86_64.linux_clang-dev/release.com
<<< Vortex OpenSplice HDE Release 6.9.210323OSS For x86_64.linux_clang-dev, Date 2022-02-23 >>>
$
```

After setting up the [OpenSplice] environments, from the root of the
`MySoftwareBus` project tree, execute the following commands:

```console
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DWITH_DDS=OpenSplice \
  -G Ninja ..
```
Once the build environment is configured, the next step is to actually run the
build:

```console
$ ninja
```

## Building The Application

The procedure that builds the application is nearly identical to that of other
examples, with the exception that the build configuration must be told where to
find the `MySoftwareBus` package. That is done in a manner very similar to how
the build system is told where to find the Foundation SDK. The environment setup
pre-requisite discussed in the
[Building Software Bus section](#building-software-bus) also applies here (the
assumptions regarding install location, version, and platform also apply):

```console
$ . /opt/VortexOpenSplice/6.9.210323OSS/HDE/x86_64.linux_clang-dev/release.com
<<< Vortex OpenSplice HDE Release 6.9.210323OSS For x86_64.linux_clang-dev, Date 2022-02-23 >>>
$
```

After the [OpenSplice] environment is set up, from the root of the
`TutorialStep7` project tree, execute the following commands:

```console
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DFoundation_DIR=${FOUNDATION_ROOT}/build/Foundation \
  -DMySoftwareBus_DIR=$(readlink -f ../../MySoftwareBus/build/MySoftwareBus) \
  -G Ninja ..
```

The command, like the previous tutorial steps, assumes the `FOUNDATION_ROOT`
environment variable is set up. The build system is told where to find the
`MySoftwareBus` package via the `MySoftwareBus_DIR` variable. The command line
leverages the `readlink` shell tool to transform the relative path to the
`MySoftwareBus` build tree to an absolute path.

> [!NOTE]
> Notice the pattern between the `Foundation_DIR` and `MySoftwareBus_DIR`
> variables? In CMake, the `find_package()` method will attempt to find the
> configuration file for the requested package in a set of well-known locations.
> When that fails (and for those custom packages, that initial search *will*
> fail), CMake tries to look in a directory pointed to by a variable following
> the name pattern `<package>_DIR` where `<package>` is the package name. Thus,
> for a CMake command that reads `find_package(Foundation ...)`, it will check
> for the presence of a `Foundation_DIR` variable. The same goes for the
> `MySoftwareBus` package.

After the build environment is configured, all that is left is to actually
build:

```console
$ ninja
```

## Running

### OpenSplice Configuration

Prior to running the application, the [OpenSplice] runtime must be configured
to host the DDS domain the applications plan to use. Assuming the applications
will use the DDS domain with identifier `30`, an [OpenSplice] configuration file
must be created to declare such a domain. From a terminal that has been prepared
with the `release.com` script as discussed in the
[Building Software Bus section](#building-software-bus), issue the following
command to create a copy of the baseline configuration file:

```console
$ curl ${OSPL_URI} > /tmp/ospl.xml
```

> [!NOTE]
> Why use `curl` to read the file?
>
> Examining the contents of the `OSPL_URI` environment variable reveals that the
> configuration file is identified using a `file:` uniform resource identifier
> (URI). Common file system tools would not understand the format, but `curl`
> simply treats it like any other resource.

Next, open the XML-formatted configuration file and alter the value contained in
the first `Id` tag inside the first `Domain` tag inside the root `OpenSplice`
tag so that it reads `30` (the equivalent [XPath][xpath_intro] expression would
be: `/OpenSplice/Domain[1]/Id[1]`):

```diff
<OpenSplice>
    <Domain>
        <Name>ospl_sp_ddsi</Name>
-       <Id>0</Id>
+       <Id>30</Id>
```

> [!NOTE]
> The choice of `30` for the domain identifier is arbitrary. Any integer value
> in the range `[0, 232]` should work, although unintended cross-talk between
> two developers using the same domain is possible.

There are many other settings available for customization in the aforementioned
configuration file, but for this tutorial the focus will be on the domain
identifier.

### Running The Subscriber

From another terminal shell session prepared the same way as that one in the
[Running The Publisher section](#running-the-publisher), execute the application
as follows:

```console
$ env OSPL_URI=file:///tmp/ospl.xml ./SoftwareBusPrimer --dds-domain=30 --subscriber
```

At startup, the application will at most indicate that the DDS system has been
activated with a `Started DDS Participant` log message, but will not report any
other activity.

> [!NOTE]
> Why preface the call to `SoftwareBusPrimer` with the `env` tool?
>
> The [OpenSplice Configuration](#opensplice-configuration) section showed how
> to copy the baseline configuration file for customization by using the
> `OSPL_URI` environment variable. As it turns out, [OpenSplice] uses that
> environment variable internally as well when its initialized. Thus, in order
> to provide the [OpenSplice] library runtime with the customized configuration
> file, it is necessary to alter the value of the `OSPL_URI` environment
> variable. Instead of altering the variable for the entire terminal shell
> session (in Bourne/BASH/Korn/ZSH shell that would be with the `export`
> statement), this example chose to customize just the environment that the
> application will use as it runs.

### Running The Publisher

From a terminal shell session that has been prepared with the [OpenSplice]
`release.com` setup script and a current working directory in the
`TutorialStep7` build tree, execute the application as follows:

```console
$ env OSPL_URI=file:///tmp/ospl.xml ./SoftwareBusPrimer --dds-domain=30
```

The application should start after a short while and begin reporting on the
samples it sends:

```
[2024-06-07T14:09:30.426Z] [SoftwareBusPrimer] [108419] [INFORMATION]: Started DDS Participant "SoftwareBusPrimer[Publisher]" on domain (30).
[2024-06-07T14:09:31.428Z] [SoftwareBusPrimer] [108419] [INFORMATION]: Sending sample: { "alpha": 83, "bravo": 86, "charlie": 77 }
[2024-06-07T14:09:32.428Z] [SoftwareBusPrimer] [108419] [INFORMATION]: Sending sample: { "alpha": 15, "bravo": 93, "charlie": 35 }
[2024-06-07T14:09:33.428Z] [SoftwareBusPrimer] [108419] [INFORMATION]: Sending sample: { "alpha": 86, "bravo": 92, "charlie": 49 }
[2024-06-07T14:09:34.428Z] [SoftwareBusPrimer] [108419] [INFORMATION]: Sending sample: { "alpha": 21, "bravo": 62, "charlie": 27 }
[2024-06-07T14:09:35.428Z] [SoftwareBusPrimer] [108419] [INFORMATION]: Sending sample: { "alpha": 90, "bravo": 59, "charlie": 63 }
```

Back in the terminal window where the subscribing application was started, more
log message activity evinces receipt of samples transmitted by the publishing
application:

```
[2024-06-07T14:09:29.821Z] [SoftwareBusPrimer] [108323] [INFORMATION]: Started DDS Participant "SoftwareBusPrimer[Subscriber]" on domain (30).
[2024-06-07T14:09:31.429Z] [SoftwareBusPrimer] [108323] [INFORMATION]: Received sample: { "alpha": 83, "bravo": 86, "charlie": 77 }
[2024-06-07T14:09:32.429Z] [SoftwareBusPrimer] [108323] [INFORMATION]: Received sample: { "alpha": 15, "bravo": 93, "charlie": 35 }
[2024-06-07T14:09:33.429Z] [SoftwareBusPrimer] [108323] [INFORMATION]: Received sample: { "alpha": 86, "bravo": 92, "charlie": 49 }
[2024-06-07T14:09:34.429Z] [SoftwareBusPrimer] [108323] [INFORMATION]: Received sample: { "alpha": 21, "bravo": 62, "charlie": 27 }
[2024-06-07T14:09:35.429Z] [SoftwareBusPrimer] [108323] [INFORMATION]: Received sample: { "alpha": 90, "bravo": 59, "charlie": 63 }
```

## Conclusion

Congratulations! 🎉🎉 You have created a Foundation application that communicates
over a DDS-based software bus with other software components. Although the
actual application design is quite simple, the discussion for this example is
quite long as it introduces several new concepts. The next tutorial steps will
build upon this success to eventually create a small system of software
components that can emulate command and control of a device.


[OpenSplice]: https://github.com/ADLINK-IST/opensplice "Vortex OpenSplice Community Edition"
[omg_dds_spec]: https://www.omg.org/spec/DDS/ "OMG DDS Specification"
[omg_dds_xml_spec]: https://www.omg.org/spec/DDS-XML/ "OMG DDS XML Syntax Specification"
[omg_idl_spec]: https://www.omg.org/spec/IDL/ "OMG IDL Specification"
[xpath_intro]: https://www.w3schools.com/xml/xpath_intro.asp "W3Schools XPath Introduction"
