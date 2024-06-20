# Tutorial Step 8: Instrument Agent Primer

Coming to the end of this tutorial journey, this final example will bring
together several of the concepts introduced in prior tutorial examples to
implement an application that represents a device before the common software
bus. In other words, a very basic instrument agent. Since the agent existing in
isolation is not very useful, it will be combined with pre-built components that
bookend a software system. The following diagram depicts the overall
architecture of the software system the agent will exist in:

```mermaid
block-beta
  vcan_dev["Virtual CAN\nTarget"] space:1 agent("Instrument\nAgent") space:1 cpanel["Control\nPanel"]
  vcan_dev-- "CAN\nbus" -->agent
  agent-- "CAN\nbus" -->vcan_dev
  agent-- "Software\nBus" -->cpanel
  cpanel-- "Software\nBus" -->agent
```

## Provided Components

The **Virtual CAN Target** and **Control Panel** components will be provided and
ready to use. Their individual purpose in the system is as follows:

* The **Virtual CAN Target** will act as a device on the CAN bus, using the
  Virtual CAN bus used in [Step 4](./step_4.md) of the tutorial. It will
  transmit CAN frames for three (3) numeric values and will accept commands for
  three (3) discrete switches. The device will have a text-based user interface
  serving as a "front panel" for developers to alter outgoing values and monitor
  incoming commands.
* The **Control Panel** will act as a Software Bus-based user interface
  application that will monitor the virtual CAN device numeric values (i.e., the
  "telemetry") as translated by the **Instrument Agent** and will issue commands
  over the software bus for the agent to translate for the virtual CAN device.
  The user interface will be text-based.

## Target Dialect

### CAN-bus Target Interface

The **Virtual CAN Target** will emit the following numeric values over CAN bus,
each in its own CAN frame, under the indicated CAN arbitration IDs (CAN ID for
short):

| CAN ID  | Format         | DLC | Range      |
| :------ | :------------- | :-- | :--------- |
| `0x100` | unsigned 8-bit | 1   | `[0, 100]` |
| `0x101` | unsigned 8-bit | 1   | `[0, 100]` |
| `0x102` | unsigned 8-bit | 1   | `[0, 100]` |

The device will also accept discrete commands destined for virtual ON/OFF
actuators in CAN frames identified by the following CAN IDs:

| CAN ID  | Format         | DLC | Value    | Action   |
| :------ | :------------- | :-- | :------- | :------- |
| `0x200` | unsigned 8-bit | 1   | `!= 255` | Turn OFF |
| `0x200` | unsigned 8-bit | 1   | `== 255` | Turn ON  |
| `0x201` | unsigned 8-bit | 1   | `!= 255` | Turn OFF |
| `0x201` | unsigned 8-bit | 1   | `== 255` | Turn ON  |
| `0x202` | unsigned 8-bit | 1   | `!= 255` | Turn OFF |
| `0x202` | unsigned 8-bit | 1   | `== 255` | Turn ON  |

> [!NOTE]
> Why is the distribution of values between command states so lopsided?
>
> It would have been easy enough to assign value `0` as OFF and `1` as ON.
> However, in hardware interface design there are times when a specific
> actuator nominal (or "safe") position is shown preference. This example
> assumes that the switch OFF position is the safe, nominal state. Thus, the
> interface design requiring that moving the actuator to the ON position involve
> receiving a very specific value. Other value distributions would be just as
> valid, and would depend on the targeted hardware.

### Software Bus Interface

In the instrument agent, the above values will be translated by assigning
mnemonics (or "labels") to each. Those assigned labels will be featured in the
Software Bus-based interface the agent offers:

| CAN ID  | Label     |
| :------ | :-------- |
| `0x100` | `alpha`   |
| `0x101` | `bravo`   |
| `0x102` | `charlie` |
| `0x200` | `sw0`     |
| `0x201` | `sw1`     |
| `0x202` | `sw2`     |

Fresh off of completing [Step 7](./step_7.md) of the tutorial, the names
`alpha`, `bravo`, and `charlie` (other than being part of the aeronautical
phonetic alphabet) should seem familiar: they are the names used in the
`EngineeringData` message structure in the
[software bus topic configuration](./step_7.md#software-bus-topic-configuration).
Indeed, one of the primary tasks of the instrument agent will be to collect
(or "acquire") incoming values over the aforementioned CAN IDs, collate them,
and emit them in `EngineeringData` topic samples.

The commands are not as obvious of a connection, since the `Command` topic
message structure only bears a character string field called `statement`. The
way the instrument agent will map the labels to the command messages is by
defining a very simple grammar describing the command text structure:

```bnf
<statement> ::= <target> <action>
<target>    ::= "sw0" | "sw1" | "sw2"
<action>    ::= "OFF" | "ON"
```

Thus, based on that grammar, a command statement that turns off `sw0` would look
like:

```
sw0 OFF
```

A command statement that turn on `sw1` would look like:

```
sw1 ON
```

## Supporting Library Bootstrap

Both the **Virtual CAN Target** and **Control Panel** applications provide a
text-based user interface implemented using the [NCURSES][ncurses_doc] library.
To more efficiently implement the applications, a separate utility library was
written that simplifies using [NCURSES][ncurses_doc]. Copy the code for this
library to the folder containing all other tutorial examples as follows (this
assumes the `FOUNDATION_ROOT` environment variable contains the location to the
Foundation project tree):

```console
$ rsync -auv ${FOUNDATION_ROOT}/examples/NcursesKit .
```

## Virtual CAN Target Bootstrap

The code that implements the **Virtual CAN Target** is already provided in the
Foundation project tree and can be copied over to the directory containing all
other tutorial examples as follows (this assumes the `FOUNDATION_ROOT`
environment variable contains the location to the Foundation project tree):

```console
$ rsync -auv ${FOUNDATION_ROOT}/examples/VirtualCanTarget .
```

## Control Panel Bootstrap

The code that implements the **Control Panel** is already provided in the
Foundation project tree and can be copied over to the directory containing all
other tutorial examples as follows (this assumes the `FOUNDATION_ROOT`
environment variable contains the location to the Foundation project tree):

```console
$ rsync -auv ${FOUNDATION_ROOT}/examples/ControlPanel .
```

## Example Application Bootstrap

The instrument agent that will be built in this tutorial step bears enough of
a resemblance to the application written in [Step 7](./step_7.md) that its
starting point can be where Step 7 finished. From the directory containing all
other tutorial examples, most importantly the `TutorialStep7` and
`MySoftwareBus` project directories, execute the following:

```console
$ mkdir TutorialStep8
$ cd TutorialStep8
$ rsync -auv --exclude 'build/' --exclude 'compile_commands.json' ../TutorialStep7/ .
```

## NCurses Library Build

Normally, building of code is left to the end of the example steps, but the
packages that support the example application can be built "a-priori." For the
[NCURSES][ncurses_doc] library, starting from the root folder as copied over in
[the corresponding bootstrapping section](#supporting-library-bootstrap),
execute the following:

```console
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++ -G Ninja ..
$ ninja
```

## Virtual CAN Target Build

Starting from the root folder as copied over when the
[**Virtual CAN Target** project tree was bootstrapped](#virtual-can-target-bootstrap),
execute the following (this assumes the `FOUNDATION_ROOT` environment variable
contains the location to the Foundation project tree and that the [OpenSplice]
environment setup script has been sourced):

```console
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DNcursesKit_DIR=$(readlink -f ../../NcursesKit/build/NcursesKit) \
  -DFoundation_DIR=${FOUNDATION_ROOT}/build/Foundation \
  -G Ninja \
  ..
$ ninja
```

> [!NOTE]
> Even though the **Virtual CAN Target** does not use DDS, it links against a
> Foundation build that *does* include DDS ([OpenSplice] in this case). Thus,
> all required DDS provider environment preparation must be done prior to
> configuring the build tree. In the case of [OpenSplice], that means running
> the `release.com` script that was discussed in the
> [Step 7 Software Bus Build](./step_7.md#building-software-bus) section.

## Control Panel Build

Starting from the root folder as copied over when the
[**Control Panel** project tree was bootstrapped](#control-panel-bootstrap),
execute the following (this assumes the `FOUNDATION_ROOT` environment variable
contains the location to the Foundation project tree and that the [OpenSplice]
environment setup script has been sourced):

```console
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DNcursesKit_DIR=$(readlink -f ../../NcursesKit/build/NcursesKit) \
  -DFoundation_DIR=${FOUNDATION_ROOT}/build/Foundation \
  -DMySoftwareBus_DIR=$(readlink -f ../../MySoftwareBus/build/MySoftwareBus) \
  -G Ninja \
  ..
$ ninja
```

## Application Project Build Configuration

Similar to the other examples, this example application must be given a unique
name. Furthermore, the project dependency list must be updated to include
support for DDS. In the `TutorialStep8` directory, edit the project metadata in
the `CMakeLists.txt` `project()` command as follows:

```diff
- project(SampleApp LANGUAGES CXX)
+ project(VctAgent LANGUAGES CXX)
```

Then, update the project dependencies in the `find_package()` command:

```diff
-find_package(Foundation REQUIRED CoreKit DdsKit)
+find_package(Foundation REQUIRED COMPONENTS CoreKit CanBusKit DdsKit)
+find_package(MySoftwareBus REQUIRED)
```

Finally, include the required components in the `target_link_libraries()`
command as follows:

```diff
-target_link_libraries(${PROJECT_NAME} Foundation::CoreKit Foundation::DdsKit MySoftwareBus::MySoftwareBus)
+target_link_libraries(${PROJECT_NAME} Foundation::CoreKit Foundation::CanBusKit Foundation::DdsKit MySoftwareBus::MySoftwareBus)
```

## Application Main Function

There are very little changes required of the main function, just like in
[Step 1](./step_1.md). The change involves altering the application name for
logging purposes in the call to `::CoreKit::Application::initialize()`. From the
`TutorialStep8` project folder modify the `src/main.cpp` file as follows:

```diff
- G_MyApp->initialize("SoftwareBusPrimer", argc, argv);
+ G_MyApp->initialize("VctAgent", argc, argv);
```

## Application Delegate

### Class Structure Customization

Normally, on anything but a basic application, a lot of the logic is written
away from the application delegate (as shown in [Step 6](./step_6.md)). In this
case, however, to keep things easy to explain, all of the custom application
logic will remain in the delegate class. From the `TutorialStep8` project root
folder, make the following changes to the top of the `MyAppDelegate` class
definition in the `src/MyAppDelegate.h` file:

```diff
#include <memory>
#include <string>
#include <CoreKit/CoreKit.h>
+#include <CanBusKit/CanBusKit.h>
#include <DdsKit/DdsKit.h>
+#include <CommandConfig.h>
#include <EngineeringDataConfig.h>

class MyAppDelegate : public CoreKit::AppDelegate, public CoreKit::InterruptListener
{
```

Next, towards the end of the `MyAppDelegate` class definition, make the
following changes in the `src/MyAppDelegate.h` file:

```diff
    virtual void applicationDidTerminate(CoreKit::Application *theApp) override;

    virtual void timerExpired(int timerId) override;

-   void onEngineeringData(MySoftwareBus::EngineeringData *sample);
+   void onCanBusFrame(CanBusKit::CanBusFrameNotification *canNotif);
+
+   void onCommand(MySoftwareBus::Command *cmdSample);

private:
+   std::unique_ptr< CanBusKit::CanBusIo > m_canIf;
+   std::string m_canIfName;
+   DdsKit::ConfiguredDdsTopic< CommandConfig > *m_commandTopic;
+   MySoftwareBus::EngineeringData m_currentValues;
    int m_ddsDomainId;
    DdsKit::ConfiguredDdsTopic< EngineeringDataConfig > *m_engrDataTopic;
    std::unique_ptr< DdsKit::BasicDdsParticipant > m_participant;
-   bool m_subscriberMode;
    int m_timerId;
};
```

Both the set of changes shown accomplish the following:

* Alter the application definition so that instead of preparing to receive
  `MySoftwareBus::EngineeringData` samples from the software bus, it prepares
  to receive `MySoftwareBus::Command` samples, presumably from the
  **Control Panel** application (although it can be from any software bus
  participant): the `onCommand()` method.
* Prepare to receive CAN bus frames, presumably from the **Virtual CAN Target**
  application: the `onCanBusFrame()` method.
* Add an instance field definition for the CAN bus interface: `m_canIf` field.
* Add an instance field to hold the name of the CAN bus interface: `m_canIfName`
  field.
* Add an instance field definition for the `MySoftwareBus::Command` topic:
  `m_commandTopic` field.
* Create an instance field to hold the last observed values for all data points:
  `m_currentValues` field.

### Implementation File Preamble

In order to prepare the implementation of the `MyAppDelegate` class, modify the
following content at the beginning of the `src/MyAppDelegate.cpp` file:

```diff
+#include <algorithm>
#include <cstdlib>
#include <functional>
+#include <iterator>
+#include <regex>
#include <sstream>
#include <stdexcept>
#include <vector>
#include "MyAppDelegate.h"


+using CanBusKit::CanBusFrameNotification;
+using CanBusKit::CanBusIo;
+using CanBusKit::newCanBusFrameCallback;
using CoreKit::AppLog;
using CoreKit::Application;
using CoreKit::EndLog;
using CoreKit::G_MyApp;
using DdsKit::BasicDdsParticipant;
using DdsKit::ConfiguredDdsTopic;
using DdsKit::newCallbackForTopic;
+using MySoftwareBus::Command;
using MySoftwareBus::EngineeringData;

namespace
{
+   // Command-Line Flag Names
+   const char *CAN_IF_FLAG = "can-if";
    const char *DOMAIN_ID_FLAG = "dds-domain";
-   const char *SUBSCRIBER_FLAG = "subscriber";
+   // CAN ID definitions
+   constexpr canid_t ALPHA_CAN_ID = 0x100;
+   constexpr canid_t BRAVO_CAN_ID = 0x101;
+   constexpr canid_t CHARLIE_CAN_ID = 0x102;
+   constexpr canid_t SW0_CAN_ID = 0x200;
+   constexpr canid_t SW1_CAN_ID = 0x201;
+   constexpr canid_t SW2_CAN_ID = 0x202;
}
```

The above changes add all necessary namespace imports and customize the
command line flag names to be able to receive the name of the CAN bus interface
to use. The changes also add constant definitions for the CAN bus IDs that will
be used to interact with the **Virtual CAN Target**.

Next, modify the constructor implementation for the `MyAppDelegate` class as
follows in the `src/MyAppDelegate.cpp` file:

```diff
MyAppDelegate::MyAppDelegate():
+   m_commandTopic(nullptr),
    m_ddsDomainId(-1),
    m_engrDataTopic(nullptr),
-   m_subscriberMode(false),
    m_timerId(-1)
{
+   m_currentValues.alpha = 0;
+   m_currentValues.bravo = 0;
+   m_currentValues.charlie = 0;
}
```

The changes to the constructor simply initialize the values of the instance
field to nominal.

### Command Line Configuration

After that, alter the implementation of the `configureCmdLineArgs()` method in
the `src/MyAppDelegate.cpp` file as follows:

```diff
void
MyAppDelegate::configureCmdLineArgs(Application *theApp)
{
+   theApp->addCmdLineArgDef(
+       Application::CmdLineArg(
+           CAN_IF_FLAG,
+           true,
+           "Name of the CAN bus interface to use."
+       )
+   );
    theApp->addCmdLineArgDef(
        Application::CmdLineArg(
            DOMAIN_ID_FLAG,
            true,
            "Identifier of the DDS domain to participate in."
        )
    );
-   theApp->addCmdLineArgDef(
-       Application::CmdLineArg(
-           SUBSCRIBER_FLAG,
-           false,
-           "If present, start the application in subscriber mode; start in publisher mode if absent."
-       )
-   );
    BasicDdsParticipant::AddDdsRelatedCmdArgs(theApp);
}
```

The above changes configure the `--can-if` command-line flag to accept the name
of the CAN bus interface the application will use.

### Application Initialization

Next, alter the application initialization method `applicationDidInitialize()`
as follows in the `src/MyAppDelegate.cpp` class:

```diff
void
MyAppDelegate::applicationDidInitialize(Application *theApp)
{
    std::vector< std::string > ddsProviderArgs;
-   std::stringstream participantName;

+   m_canIfName = theApp->getCmdLineArgFor(CAN_IF_FLAG);
+   if (m_canIfName.empty())
+   {
+       throw std::runtime_error("Must specify a CAN bus interface name.");
+   }
+   m_canIf.reset(new CanBusIo(m_canIfName, theApp->getMainRunLoop()));
    m_ddsDomainId = std::stoi(theApp->getCmdLineArgFor(DOMAIN_ID_FLAG));
-   m_subscriberMode = !(theApp->getCmdLineArgFor(SUBSCRIBER_FLAG).empty());
    ddsProviderArgs = BasicDdsParticipant::GatherDdsRelatedCmdArgs(theApp);

-   participantName << theApp->log().appName() << "[" << (m_subscriberMode ? "Subscriber" : "Publisher") << "]";
    m_participant.reset(
        new BasicDdsParticipant(
            m_ddsDomainId,
-           participantName.str(),
+           theApp->log().appName(),
            ddsProviderArgs,
            &theApp->log()
        )
    );
}
```

The above changes process the incoming CAN bus interface name via the command
line and remove the logic that was originally put in to implement the different
operating modes of [Step 7](./step_7.md). The interface name is then used to
create the `CanBusKit::CanBusIo` instance that will be used to exchange messages
(frames) over the CAN bus.

### Application Start

Alter the `MyAppDelegate::applicationStarting()` method in the
`src/MyAppDelegate.cpp` file as follows:

```diff
void
MyAppDelegate::applicationStarting(Application *theApp)
{
+   if (!m_canIf)
+   {
+       throw std::runtime_error("Attempting to start without CAN bus interface.");
+   }
+   m_canIf->addCanFrameCallback(
+       newCanBusFrameCallback(
+           std::bind(
+               std::mem_fn(&MyAppDelegate::onCanBusFrame),
+               this,
+               std::placeholders::_1
+           )
+       )
+   );
+   m_canIf->startCan();
    if (!m_participant)
    {   
        throw std::runtime_error("Attempting to start without DDS domain participant.");
    }
    m_participant->startDds();
    // Although the topic object is created here, it is "adopted" and
    // subsequently managed by the BasicDdsParticipant instance.
+   m_commandTopic = new ConfiguredDdsTopic< CommandConfig >(
+       theApp->getMainRunLoop(),
+       m_participant.get()
+   );
    m_engrDataTopic = new ConfiguredDdsTopic< EngineeringDataConfig >(
        theApp->getMainRunLoop(),
        m_participant.get()
    );
    
+   m_commandTopic->createReader(CommandConfig::NormallyReliable);
+   m_commandTopic->registerCallback(
+       newCallbackForTopic(
+           CommandConfig(),
+           std::bind(
+               std::mem_fn(&MyAppDelegate::onCommand),
+               this,
+               std::placeholders::_1
+           )
+       )
+   );
+   m_commandTopic->listenForSamples();
+   m_engrDataTopic->createWriter(EngineeringDataConfig::NormallyReliable);
+   m_timerId = theApp->getMainRunLoop()->registerTimerWithInterval(1.0, this, true);
-   if (m_subscriberMode)
-   {   
-       m_engrDataTopic->createReader(EngineeringDataConfig::NormallyReliable);
-       m_engrDataTopic->registerCallback(
-           newCallbackForTopic(
-               EngineeringDataConfig(),
-               std::bind(
-                   std::mem_fn(&MyAppDelegate::onEngineeringData),
-                   this,
-                   std::placeholders::_1
-               )
-           )
-       );
-       m_engrDataTopic->listenForSamples();
-   }
-   else
-   {
-       m_engrDataTopic->createWriter(EngineeringDataConfig::NormallyReliable);
-       m_timerId = theApp->getMainRunLoop()->registerTimerWithInterval(1.0, this, true);
-   }
}
```

The above changes again mostly remove the logic that allowed the test
application written in [Step 7](./step_7.md) to run in two (2) modes. This
application firmly operates in a single mode: accepting `MySoftwareBus::Command`
samples from the software bus and transmitting `MySoftwareBus::EngineeringData`
samples. The data samples are published at the beat of a timer, also set up in
this method logic. The method also enables the CAN bus interface and registers
an event handler for any incoming CAN bus frames.

> [!NOTE]
> Why not publish `EngineeringData` every time there's an update?
>
> It is certainly possible to alter the design of the application so that every
> time a new data point arrives via the CAN bus, the example agent application
> emits a new `EngineeringData` sample. In this particular case, however, since
> the target device emits *all* data points as a batch on the regular, sending
> a new `EngineeringData` sample carrying all three (3) data points when only
> a fresh sample for one was received would be wasteful. Alternative logic would
> either have the agent wait for fresh samples on all data points before
> emitting an `EngineeringData` sample of its own. Yet another alternative would
> have the agent only emitting `EngineeringData` samples whenever a
> *value change* (not necessarily a fresh sample) is observed. All alternatives
> bring with them a set of pros and cons that application and system designers
> will need to consider.

### Application Pre-Termination

Replace the `applicationWillTerminate()` method implementation in the
`src/MyAppDelegate.cpp` file with the following content:

```c++
void
MyAppDelegate::applicationWillTerminate(Application *theApp)
{
    if (m_commandTopic != nullptr)
    {
        m_commandTopic->stopListening();
    }
    if (m_timerId != -1)
    {
        theApp->getMainRunLoop()->deregisterTimer(m_timerId);
        m_timerId = -1;
    }
    if (m_participant)
    {
        m_participant->stopDds();
        m_commandTopic = nullptr;
        m_engrDataTopic = nullptr;
    }
    if (m_canIf)
    {
        m_canIf->stopCan();
    }
}
```

Just like in previous examples, this method logic simply tears down any active
components in the application in preparation for shutdown.

### Application Post-Termination

Similar to [Step 7](./step_7.md), once the main run loop exits the only thing
left to do is to release any resources. Alter the implementation of the
`applicationDidTerminate()` method in the `src/MyAppDelegate.cpp` file as
follows:

```diff
void
MyAppDelegate::applicationDidTerminate(Application *theApp)
{
    m_participant.reset(nullptr);
+   m_canIf.reset(nullptr);
}
```

### Timer Expiration Handler

The example agent is designed to publish `EngineeringData` at regular intervals
based on a timer. The method publishes a new sample whether it received fresh
data over the CAN bus or not. Replace the `timerExpired()` method implementation
in the `src/MyAppDelegate.cpp` file with the following:

```c++
void
MyAppDelegate::timerExpired(int timerId)
{
    G_MyApp->log() << AppLog::LL_INFO << "Sending sample: " << EngineeringDataToString(&m_currentValues) << EndLog;
    m_engrDataTopic->writeSample(&m_currentValues);
}
```

### CAN Bus Data Handler

The agent will receive fresh samples for the target's data points via CAN bus
notifications, which will arrive at the `onCanBusFrame()` method. Add the
following implementation to it in the `src/MyAppDelegate.cpp` file:

```c++
void
MyAppDelegate::onCanBusFrame(CanBusFrameNotification *canNotif)
{
    switch(canNotif->canId)
    {
    case ALPHA_CAN_ID:
        m_currentValues.alpha = canNotif->canPayload[0];
        break;
    
    case BRAVO_CAN_ID:
        m_currentValues.bravo = canNotif->canPayload[0];
        break;
    
    case CHARLIE_CAN_ID:
        m_currentValues.charlie = canNotif->canPayload[0];
        break;
    }
}
```

The implementation for incoming CAN bus data simply stores the incoming sample
into the cache that will be used to publish an `EngineeringData` sample in the
[timer expiration method](#timer-expiration-handler).

> [!NOTE]
> In order to keep things simple, this code is quite cavalier as it comes to
> accepting samples from the device. A more robust application would have the
> sample ingest perform some cursory checks (e.g., are there any flags set, is
> the payload the correct size for the ID, etc.).

### Software Bus Command Handler

Another job of the agent is to translate commands arriving from the software bus
to messages that the target will understand. Add the following implementation
for the `onCommand()` method in the `src/MyAppDelegate.cpp` class:

```c++
void
MyAppDelegate::onCommand(Command *cmdSample)
{
    static const std::map< std::string, canid_t > TARGETS(
        {
            { "sw0", SW0_CAN_ID },
            { "sw1", SW1_CAN_ID },
            { "sw2", SW2_CAN_ID }
        }
    );
    static const std::map< std::string, std::uint8_t > ACTIONS(
        {
            { "OFF", 0u },
            { "ON", 255u }
        }
    );

    struct can_frame canCmd;

    memset(&canCmd, 0x00, sizeof(can_frame));

    std::regex whitespaceSep("\\s+");
    std::vector< std::string > commandTokens;
    std::string commandStatement = DdsKit::GetStringContents(cmdSample->statement);
    std::copy(
        std::sregex_token_iterator(
            commandStatement.begin(),
            commandStatement.end(),
            whitespaceSep,
            -1
        ),
        std::sregex_token_iterator(),
        std::back_inserter(commandTokens)
    );
    if (commandTokens.size() != 2u)
    {
        G_MyApp->log() << AppLog::LL_WARNING
            << "Not enough tokens in command \""
            << commandStatement
            << "\". Expecting two (2)"
            << EndLog;
        return;
    }
    auto targetIter = TARGETS.find(commandTokens[0]);
    if (TARGETS.end() == targetIter)
    {
        G_MyApp->log() << AppLog::LL_WARNING
            << "Unknown command target \""
            << commandTokens[0]
            << "\"."
            << EndLog;
        return;
    }
    auto actionIter = ACTIONS.find(commandTokens[1]);
    if (ACTIONS.end() == actionIter)
    {
        G_MyApp->log() << AppLog::LL_WARNING
            << "Unknown command action \""
            << commandTokens[1]
            << "\"."
            << EndLog;
        return;
    }

    G_MyApp->log() << AppLog::LL_INFO
        << "Processing command statement: "
        << commandStatement
        << EndLog;
    canCmd.can_id = (*targetIter).second;
    canCmd.can_dlc = 1;
    canCmd.data[0] = (*actionIter).second;

    m_canIf->sendCanFrame(&canCmd);
}
```

The implementation of the handler certainly looks long, but when broken down
by work done it is not difficult to cover:

* Using the C++11 regular expression library, the method breaks down the command
  string per the grammar specified in the
  [Software Bus Interface](#software-bus-interface) section.
* If either the command target or the action are unrecognized, a warning message
  is added to the log and the method exits.
* Otherwise, the target is translated to its appropriate CAN ID and the action
  to its appropriate frame payload content.
* Then, the CAN bus frame is sent out.

> [!NOTE]
> What's with the `DdsKit::GetStringContents()`?
>
> Although the DDS Traditional C++ Platform-Specific Model (PSM) standard is
> quite prescriptive as far as defining the mapping between the abstract API
> and C++, it does not specify everything. In particular, it does not specify
> how character strings are mapped to C++. Some DDS providers chose to map them
> to C/C++ `char*` instances, others prefer to use the C++ `std::string` object,
> while others (like [OpenSplice]) chose to create their own character string
> "management" class ([OpenSplice] calls theirs `DDS::String_mgr`). Thus, to
> increase portability between DDS providers, the Foundation `DdsKit` provides
> targeted utilities, such as `DdsKit::GetStringContents()` that handles the
> peculiarities for the developer.

### Engineering Data Handler Remove

The delegate implementation in [Step 7](./step_7.md#incoming-sample-handler)
added an event handler for incoming `EngineeringData` samples, but this
application does not listen for those, so that implementation must be removed
from the `src/MyAppDelegate.cpp` file:

```diff
-void
-MyAppDelegate::onEngineeringData(EngineeringData *sample)
-{
-   G_MyApp->log() << AppLog::LL_INFO << "Received sample: " << EngineeringDataToString(sample) << EndLog;
-}
```

## Building The Application

Just like the
[application build process in Step 7](./step_7.md#building-the-application),
this example application must be instructed where to find the `MySoftwareBus`
package. Also, just like when
[building the **Virtual CAN Target**](#virtual-can-target-build), the terminal
shell environment to use must have already been prepared with the [OpenSplice]
preparatory script `release.com`. With the proper shell environment, from the
`TutorialStep8` project directory, execute the following statements to configure
the application build tree:

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

> [!NOTE]
> A clear indication that the [OpenSplice] environment preparation step has not
> run when attempting to configure the build tree is if the configuration fails
> with an error that reads something like:
>
> `The 'OSPL_HOME' environment variable is not properly configured.`

After the build tree is set up, run the build as follows:

```console
$ ninja
```

## Running

### Running the Example Application

The example application is able to run on its own without any other entities
present either in the CAN bus or in the software bus, although it does not do
anything interesting like that; it merely provides regular 1Hz updates on
unchanging data points:

```console
$ env OSPL_URI=file:///tmp/ospl.xml ./VctAgent --can-if vcan0 --dds-domain 30
[2024-06-11T14:21:58.547Z] [VctAgent] [579088] [INFORMATION]: Started DDS Participant "VctAgent" on domain (30).
[2024-06-11T14:21:59.551Z] [VctAgent] [579088] [INFORMATION]: Sending sample: { "alpha": 0, "bravo": 0, "charlie": 0 }
[2024-06-11T14:22:00.551Z] [VctAgent] [579088] [INFORMATION]: Sending sample: { "alpha": 0, "bravo": 0, "charlie": 0 }
[2024-06-11T14:22:01.551Z] [VctAgent] [579088] [INFORMATION]: Sending sample: { "alpha": 0, "bravo": 0, "charlie": 0 }
[2024-06-11T14:22:02.551Z] [VctAgent] [579088] [INFORMATION]: Sending sample: { "alpha": 0, "bravo": 0, "charlie": 0 }
[2024-06-11T14:22:03.551Z] [VctAgent] [579088] [INFORMATION]: Sending sample: { "alpha": 0, "bravo": 0, "charlie": 0 }
[2024-06-11T14:22:04.551Z] [VctAgent] [579088] [INFORMATION]: Sending sample: { "alpha": 0, "bravo": 0, "charlie": 0 }
```

### Running a Subscribing Entity

The example application used in [Step 7](./step_7.md#running-the-subscriber), in
subscriber mode, can be used to confirm proper `EngineeringData` publishing,
though. With the `VctAgent` application running in its own terminal window, open
a new terminal shell session, prepare it with the [OpenSplice] environment, and
run the example application in subscriber mode to verify the software bus
traffic:

```console
$ env OSPL_URI=file:///tmp/ospl.xml ./SoftwareBusPrimer --dds-domain=30 --subscriber
[2024-06-11T14:29:41.029Z] [SoftwareBusPrimer] [579326] [INFORMATION]: Started DDS Participant "SoftwareBusPrimer[Subscriber]" on domain (30).
[2024-06-11T14:30:01.759Z] [SoftwareBusPrimer] [579326] [INFORMATION]: Received sample: { "alpha": 0, "bravo": 0, "charlie": 0 }
[2024-06-11T14:30:02.758Z] [SoftwareBusPrimer] [579326] [INFORMATION]: Received sample: { "alpha": 0, "bravo": 0, "charlie": 0 }
[2024-06-11T14:30:03.758Z] [SoftwareBusPrimer] [579326] [INFORMATION]: Received sample: { "alpha": 0, "bravo": 0, "charlie": 0 }
[2024-06-11T14:30:04.758Z] [SoftwareBusPrimer] [579326] [INFORMATION]: Received sample: { "alpha": 0, "bravo": 0, "charlie": 0 }
[2024-06-11T14:30:05.758Z] [SoftwareBusPrimer] [579326] [INFORMATION]: Received sample: { "alpha": 0, "bravo": 0, "charlie": 0 }
[2024-06-11T14:30:06.758Z] [SoftwareBusPrimer] [579326] [INFORMATION]: Received sample: { "alpha": 0, "bravo": 0, "charlie": 0 }
[2024-06-11T14:30:07.758Z] [SoftwareBusPrimer] [579326] [INFORMATION]: Received sample: { "alpha": 0, "bravo": 0, "charlie": 0 }
```

### Running the Virtual CAN Target

When paired with the **Virtual CAN Target**, however, things start to look a
little more interesting. In yet another terminal window, start the
`VirtualCanTarget` application by issuing the command:

```console
$ ./VirtualCanTarget --can-if vcan0
```

The application will take over the terminal window and present a text-based user
interface showing three (3) vertical sliders labeled `ALPHA`, `BRAVO`, and
`CHARLIE`, as well as three (3) indicators labeled `SW0`, `SW1`, and `SW2`.

```

 ┌───────┐ ┌───────┐ ┌───────┐ ┌───┐
 │ ALPHA │ │ BRAVO │ │CHARLIE│ │SW0│
 │       │ │       │ │       │ └───┘
 │       │ │       │ │       │
 │       │ │       │ │       │
 │       │ │       │ │       │ ┌───┐
 │       │ │       │ │       │ │SW1│
 │       │ │       │ │       │ └───┘
 │       │ │       │ │       │
 │       │ │       │ │       │
 │       │ │       │ │       │ ┌───┐
 │       │ │       │ │       │ │SW2│
 │       │ │       │ │       │ └───┘
 │       │ │       │ │       │
 │       │ │       │ │       │
 │       │ │       │ │       │
 │       │ │       │ │       │
 │      0│ │      0│ │      0│
 └───────┘ └───────┘ └───────┘



Press [Q] to exit.
```

The vertical sliders can be used to alter the telemetry value of the data point
corresponding to its label on the CAN bus. Use the ↑ (up arrow) or ↓
(down arrow) keys to update the associated value (shown in the bottom of the
slider control). As the values in the control are updated, their corresponding
values in the CAN bus are updated as well. Navigating between the three (3)
sliders is possible via the `[TAB]` key.

The indicators reflect the last commanded state for each. Whenever the
**Virtual CAN Target** application receives a CAN message with the appropriate
CAN ID (refer to the [Target Dialect](#target-dialect) section), the indicator
will either illuminate or darken. Since the indicators are not controls, they
cannot be traversed via keyboard navigation as the vertical sliders are.

Pressing the `Q` key at any time exits the application and restores the terminal
window to its previous state.

### Running the Control Panel

Completing the solution architecture laid out
[at the introduction](#tutorial-step-8-instrument-agent-primer) is the
**Control Panel** application. In another terminal window, start the
`ControlPanel` application by issuing the command:

```console
$ env OSPL_URI=file:///tmp/ospl.xml ./ControlPanel --dds-domain=30
```

> [!NOTE]
> Since the `ControlPanel` application is a DDS application, it requires the
> `OSPL_URI` environment setup just like `VctAgent`. In a production
> environment, that variable could be set up "a-priori", obviating the need for
> the `env` preamble used up to this point.

Like the **Virtual CAN Target**, the application will take over the terminal
window and present a text-based user interface with three (3) vertical gauges
labeled `ALPHA`, `BRAVO`, and `CHARLIE`, and three (3) toggle buttons labeled
`SW0`, `SW1`, and `SW2`:

```

 ┌───────┐ ┌───────┐ ┌───────┐ ┌──────────┐
 │ ALPHA │ │ BRAVO │ │CHARLIE│ │   SW0    │
 │       │ │       │ │       │ │          │
 │       │ │       │ │       │ │ OFF  ON  │
 │       │ │       │ │       │ │          │
 │       │ │       │ │       │ └──────────┘
 │       │ │       │ │       │
 │       │ │       │ │       │ ┌──────────┐
 │       │ │       │ │       │ │   SW1    │
 │       │ │       │ │       │ │          │
 │       │ │       │ │       │ │ OFF  ON  │
 │       │ │       │ │       │ │          │
 │       │ │       │ │       │ └──────────┘
 │       │ │       │ │       │
 │       │ │       │ │       │ ┌──────────┐
 │       │ │       │ │       │ │   SW2    │
 │       │ │       │ │       │ │          │
 │      0│ │      0│ │      0│ │ OFF  ON  │
 └───────┘ └───────┘ └───────┘ │          │
                               └──────────┘


Press [Q] to exit.
```

The vertical gauges reflect the last value observed in the software bus for the
corresponding data points. The toggle buttons can be switched on and off using
the `[SPACE]` key. Navigation through the available buttons is possible via the
`[TAB]` key. Pressing `Q` will exit the application.

With the entire system up and running (`VirtualCanTarget`, `VctAgent`, and
`ControlPanel`) it is now possible to observe how the system reacts to
"telemetry" from the virtual device, and how it handles command requests. From
the terminal window running `VirtualCanTarget`, manipulate the values in the
`ALPHA`, `BRAVO`, or `CHARLIE` sliders. As the values update in the
`VirtualCanTarget` front panel they should be reflected in the terminal window
showing the `ControlPanel` application. The updates will seem to lag, and that
lag is due to the 1Hz "sampling rate" present in both the CAN target and the
agent.

Next, navigate to the terminal window running `ControlPanel` and start toggling
the buttons there. Unlike the data point "telemetry" that is reported at a fixed
interval, commands are dispatched immediately, so there should not be much (if
any) discernible lag between toggling the buttons and the `VirtualCanTarget`
front panel indicators reacting.

## Conclusion

Congratulations! 🎉🎉 You have created a Foundation-based instrument agent that
represents a device, albeit virtual, on a DDS-based software bus. The agent
translates telemetry incoming from the device via CAN bus into `EngineeringData`
software bus samples visible to other software bus participants (like
`ControlPanel` and even the `SoftwareBusPrimer` application written in
[Step 7](./step_7.md)). The agent also reacts to commands from an operator (in
this case, sent via the `ControlPanel` application) and translates them to the
proper CAN bus message for the device.

[OpenSplice]: https://github.com/ADLINK-IST/opensplice "Vortex OpenSplice Community Edition"
[ncurses_doc]: https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/ "TDLP NCURSES Programming HOWTO"
