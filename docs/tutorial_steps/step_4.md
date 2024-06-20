# Tutorial Step 4: I/O Diversification

[Step 3](./step_3.md) demonstrated two-way communication between a Foundation
application and an external entity via a serial port. As useful as that example
is, it only covers communication over a serial port. Not all devices will
communicate over a serial port, so application suites will almost certainly
need to support different external interfaces. This example shows how a
Foundation application can communicate over another popular external interface:
the Controller Area Network (CAN) bus.

Similar to the previous steps that operated with a virtual serial port, this
example will operate on a virtual CAN bus port. Most modern GNU/Linux
distributions carry with them the required kernel modules for this. Activating
the support may require a call to the `modprobe` tool:

```console
$ sudo modprobe vcan
```

Furthermore, there are a set of command-line tools that facilitate interaction
with a CAN bus port, virtual or otherwise: the
[SocketCAN Userspace Utilities package][can_utils_pkg]. The package is typically
available from major GNU/Linux package managers. In Debian/Ubuntu, the package
may be installed as follows:

```console
$ sudo apt install can-utils
```

The overall design of this example application will include a periodic timer
used to emit a message over CAN bus once a second, while also monitoring the
CAN bus for any incoming messages and emitting a log message every time a
message arrives. The message emitted once a second will be a monotonically
increasing, 32-bit integer value that increments at every timer expiration.

## Bootstrap the Project Tree

The significant differences between the serial port examples and this example
means that the best way to bootstrap the project tree is to start from the
baseline skeleton provided by the Foundation source tree, similar to
[Step 1](./step_1.md).  Giving the project the name `TutorialStep4`, with the
`FOUNDATION_ROOT` environment variable pointing to the root folder of the
Foundation tree:

```console
$ mkdir TutorialStep4
$ cd TutorialStep4
$ rsync -auv ${FOUNDATION_ROOT}/examples/SampleAppSkeleton/ .
```

The portions of the code copied over that will be customized for this tutorial
step are:

* The project build configuration file `CMakeLists.txt`
* The application main function
* The application delegate

## Project Build Configuration

Similar to the other examples, this example project must be given a unique name.
Furthermore, the project dependency list must be updated to include support for
the CAN bus. Edit the project metadata in the `CMakeLists.txt` `project()`
command as follows:

```diff
- project(SampleApp LANGUAGES CXX)
+ project(CanInteract LANGUAGES CXX)
```

Then, update the project dependencies in the `find_package()` command:

```diff
-find_package(Foundation REQUIRED CoreKit)
+find_package(Foundation REQUIRED CoreKit CanBusKit)
```

The `CanBusKit` module in the Foundation SDK brings in the portions required to
support communications over the CAN bus port. Finally, update the dependency
list for the application target in the `target_link_libraries()` command:

```diff
-target_link_libraries(${PROJECT_NAME} Foundation::CoreKit)
+target_link_libraries(${PROJECT_NAME} Foundation::CoreKit Foundation::CanBusKit)
```

## Application Main Function

There are very little changes required of the main function, just like in
[Step 1](./step_1.md). The change involves altering the application name for
logging purposes in the call to `::CoreKit::Application::initialize()` in the
`src/main.cpp` file as follows:

```diff
- G_MyApp->initialize("ApplicationName", argc, argv);
+ G_MyApp->initialize("CanInteract", argc, argv);
```

## Application Delegate

Again, the vast majority of the changes required to customize the application
lay in the application delegate.

### Application-Specific Logic

Adding support for a CAN bus port precipitates the need to add some instance
fields to the delegate class, along with some `#include` statement to import
several definitions. Alter the `src/MyAppDelegate.h` header file towards the
beginning as follows:

```diff
#ifndef _MY_APP_DELEGATE_H_
#define _MY_APP_DELEGATE_H_

+#include <cstdint>
+#include <memory>
#include <CoreKit/CoreKit.h>
+#include <CanBusKit/CanBusKit.h>
```

Since the application design includes a periodic timer along with the CAN bus
port support, there will be a need to implement the
`::CoreKit::InterruptListener` interface. Alter the `MyAppDelegate` class
declaration in `src/MyAppDelegate.h` as follows:

```diff
-class MyAppDelegate : public CoreKit::AppDelegate
+class MyAppDelegate : public CoreKit::AppDelegate, public CoreKit::InterruptListener
{
```

The application delegate class will need a method to handle the timer expiration
events as well as a method to handle the CAN bus incoming message event. Alter
the `MyAppDelegate` class definition body in `src/MyAppDelegate.h` as follows:

```diff
    virtual void applicationDidTerminate(CoreKit::Application *theApp) override;

+   virtual void timerExpired(int timerId) override;
+
+   void onCanBusInput(CanBusKit::CanBusFrameNotification* canBusInput);
```

Instances of the `CanBusKit::CanBusFrameNotification` class contain information
describing any input observed via the CAN bus port. There will be more
discussion on the class when the implementation of the `onCanBusInput()` method
is covered.

The delegate class will need to store several bits of information, like the
timer ID, the CAN bus port name, and the CAN bus device. The class will also
hold a field for the next 32-bit integer that the application will send over
the CAN bus. Alter the `MyAppDelegate` class definition body just before the
closing brace in `src/MyAppDelegate.h` as follows:

```diff
+private:
+   std::unique_ptr< CanBusKit::CanBusIo > m_canBusPort;
+   std::string m_canPortName;
+   uint32_t m_nextValue;
+   int m_timerId;
};
```

The `MyAppDelegate` implementation in `src/MyAppDelegate.cpp` needs to include
the following features:

* Add a command-line argument that lets the application user specify what
  CAN bus port to use.
* Initialization and termination of the CAN bus port.
* Registration of an event handler for CAN bus input.
* Registration of a periodic timer.
* Transmission of a CAN bus frame at every timer expiration.
* Report of any received CAN frames to the application log.

The line items involved with initialization, termination, and event handler
registration will be covered when the changes required in lifecycle methods is
covered. To start off, add the following content towards the beginning of the
`src/MyAppDelegate.cpp` class:

```diff
+#include <algorithm>
+#include <arpa/inet.h>
+#include <cstring>
+#include <functional>
#include "MyAppDelegate.h"

+using CanBusKit::CanBusIo;
+using CanBusKit::CanBusFrameNotification;
+using CanBusKit::newCanBusFrameCallback;
+using CoreKit::AppLog;
using CoreKit::Application;
+using CoreKit::EndLog;
+using CoreKit::G_MyApp;
+using CoreKit::format;

+namespace
+{
+   const char *CAN_IF_FLAG = "can-if";
+}
+
-MyAppDelegate::MyAppDelegate()
+MyAppDelegate::MyAppDelegate():
+   m_nextValue(0u),
+   m_timerId(-1)
{

}
```

There are several new additions in these changes:

* The namespace imports for several scoped names, like the `CanBusKit::CanBusIo`
  and `CanBusKit::CanBusFrameNotification` classes, allow references to those
  names in the `MyAppDelegate` implementation to be more succinct by allowing
  the omission of the containing namespace scope.
* The private global constant `CAN_IF_FLAG` defines the name of the command-line
  flag used to specify the CAN bus port to use. It is a private, or file scope,
  global constant because it is defined in an anonymous namespace.
* The field acting as a register for the next 32-bit value to emit over CAN bus
  is initialized to its first value.
* The `MyAppDelegate` constructor initializes the `m_timerId` field to a known
  value.

> [!TIP]
> Why not add those namespace imports to the header file?
>
> Although there is nothing syntactically wrong with adding import
> statements to the header file for a class, doing so forces developers that
> import the class definition via the header to accept the namespace imports as
> well. That kind of "under-the-surface" policy imposition is something that
> should be avoided in production code. The risk does not exist in this
> particular example, but it is shown nonetheless as a helpful pattern to
> follow.

Next, provide the implementation of the `configureCmdLineArgs()` method to add
support for the `can-if` flag:

```c++
void
MyAppDelegate::configureCmdLineArgs(Application *theApp)
{
    theApp->addCmdLineArgDef(
        Application::CmdLineArg(
            CAN_IF_FLAG,
            true,
            "CAN bus interface name (e.g., vcan0) to use"
        )
    );
}
```

> [!NOTE]
> This implementation differs from that first introduced in
> [Step 2](./step_2.md) in that instead of using a string literal for the
> command-line flag, a pre-defined constant is used. Since the command-line flag
> name will be referred to in more than one place in the code, it is good
> practice to centralize the definition.

After that, add the logic that will emit a CAN bus message at every timer
expiration in the implementation of the `timerExpired()` method:

```c++
void
MyAppDelegate::timerExpired(int timerId)
{
    static constexpr uint16_t CAN_ID = 0x100;
    struct can_frame canMsg;

    canMsg.can_id = CAN_ID;
    canMsg.can_dlc = sizeof(uint32_t);
    uint32_t nboVal = htonl(m_nextValue);
    memcpy(&canMsg.data[0], &nboVal, sizeof(uint32_t));
    m_canBusPort->sendCanFrame(&canMsg);
    m_nextValue++;
}
```

The implementation introduces the `struct can_frame` data structure provided by
the GNU/Linux SocketCAN interface to transact in CAN bus messages (or the more
accurate term, ["CAN frames"][can_frame_discussion]). The data structure exposes
several fields that are of interest:

* `can_id` - This field contains the so-called "arbitration ID" for the CAN
  message. The value serves as a unique identifier for a particular information
  source. The "arbitration" moniker is added to indicate the field is not just
  used for discerning between information sources but to adjudicate access to
  the bus, particularly during concurrent access attempts by multiple nodes.
  This example uses a pre-defined constant value of `0x100` (decimal 256).
* `can_dlc` - This field contains the "Data Length Code" (DLC) portion of the
  frame which indicates how big the frame payload is. This example only send a
  32-bit unsigned integer, so the DLC value is always four (4).
* `data` - This field contains the actual payload for the CAN bus frame. There
  are several [CAN bus versions][can_bus_versions], or standards, available.
  Each standard stipulates limits to the size of the frame payload. This example
  uses the **CAN 2.0B** standard, so the maximum frame size is eight (8) bytes.
  The unsigned integer sent by the application fits comfortably within this
  limit.

> [!NOTE]
> Although not strictly necessary, this example goes through the trouble of
> ensuring that the multi-byte integer value is transmitted over the CAN bus
> in "network byte order" (big endian) using the `htonl()` function. This
> convention makes it easier to integrate with devices, regardless of how the
> device manages multi-byte values internally.

With the periodic emitting logic in place, add the logic that will receive and
report on any incoming CAN bus traffic by providing the following
`onCanBusInput()` implementation:

```c++
void
MyAppDelegate::onCanBusInput(CanBusFrameNotification* canBusInput)
{
    size_t idx = 0u;

    G_MyApp->log() << AppLog::LL_DEBUG
         << "CAN Input: " << format("0x%08X", canBusInput->canId)
         << (canBusInput->effMessage()?"(EFF)":"")
         << (canBusInput->rtrMessage()?"(RTR)":"")
         << (canBusInput->errFrame()?"(ERR)":"")
         << " [" << canBusInput->canPayload.size() << "] ";
    std::for_each(
        canBusInput->canPayload.begin(),
        canBusInput->canPayload.end(),
        [](uint8_t aByte) -> void {
            G_MyApp->log() << format("%02X", aByte);
        }
    );
    G_MyApp->log() << EndLog;
}
```

The `onCanBusInput()` implementation further showcases the
`CanBusKit::CanBusFrameNotification` class. Instances of that class provide
plenty of information about the incoming CAN frame, such as:

* CAN ID value via the `canId` field.
* Frame flags via the `effMessage()`, `rtrMessage()`, and `errFrame()` methods.
* CAN data length code (DLC; i.e., the payload size) via the `canPayload.size()`
  method.
* CAN frame payload contents via the `canPayload` field.

> [!NOTE]
> Similar to the `::SerialKit::SerialDataNotification` class described in
> [Step 3](./step_3.md#application-specific-logic), instances of
> `::CanBusKit::CanBusFrameNotification` are only guaranteed valid while the
> event handler is in scope.

The method should print out a log message every time it receives a CAN bus
frame.

> [!NOTE]
> The `onCanBusInput()` implementation introduces the `CoreKit::format()`
> function used to create formatted `std::string` instances out of
> `printf()`-like arguments. The implementation also introduces several traits
> of CAN bus messages, such as the `EFF` (Extended Frame Format), `RTR` (Remote
> Transmission Request), and `ERR` (Error) flags. Explanations on all of those
> are [available online][can_frame_discussion].

### Application Initialization

Similar to the example in [Step 2](./step_2.md#application-initialization), this
example application must obtain the name of the interface device it should use
from the command-line arguments specified by the user. The application must also
allocate the object that represents the CAN bus device that will be used. Fill
out the implementation of the `applicationDidInitialize()` method in the
`src/MyAppDelegate.cpp` file with the following:

```c++
void
MyAppDelegate::applicationDidInitialize(Application *theApp)
{
    m_canPortName = theApp->getCmdLineArgFor(CAN_IF_FLAG);
    if (m_canPortName.empty())
    {
       throw std::runtime_error("No CAN bus port name provided.");
    }
 
    m_canBusPort.reset(
        new CanBusKit::CanBusIo(
            m_canPortName,
            theApp->getMainRunLoop()
        )
    );
}
```

### Application Start

Just before turning over control to the main run loop, the application must
configure and activate the following:

* The CAN bus device used for communication.
* The CAN bus input event handler.
* The periodic timer used to emit messages over CAN bus.

Provide the implementation of the `applicationStarting()` method in the
`src/MyAppDelegate.cpp` file with the following:

```c++
void
MyAppDelegate::applicationStarting(Application *theApp)
{
    m_timerId = theApp->getMainRunLoop()->registerTimerWithInterval(1.0, this, true);
	m_canBusPort->addCanFrameCallback(
        newCanBusFrameCallback(
			std::bind(
                std::mem_fn(&MyAppDelegate::onCanBusInput),
                this,
                std::placeholders::_1
            )
        )
    );
    m_canBusPort->startCan();
}
```

The registration of a member function as an event handler, particularly the use
of the `std::bind()` and `std::mem_fn()` C++11 calls, is covered in the
[Application Start discussion of Step 3](./step_3.md#application-start). The
`CanBusKit::newCanBusFrameCallback()`, referenced without its namespace due to
the `using` statement added in the
[Application-Specific Logic](#application-specific-logic) section, creates a new
event handler instance that is adopted by the `CanBusKit::CanBusIo` instance
via the class' `addCanFrameCallback()`. Thus, whenever a CAN bus message
arrives, `CanBusKit::CanBusIo` will forward the message to the event handler.
The `startCan()` method in `CanBusKit::CanBusIo` activates the CAN bus port and
enables receipt of CAN messages.

### Application Pre-Termination

As the application winds down to an orderly shutdown, the delegate must
deactivate any services driving the main run loop. For this application, that
involves stopping the periodic timer and deactivating the CAN bus port. Add the
following implementation to the `applicationWillTerminate()` method in the
`src/MyAppDelegate.cpp` file:

```c++
void
MyAppDelegate::applicationWillTerminate(Application *theApp)
{
    if (m_timerId != -1)
    {
        theApp->getMainRunLoop()->deregisterTimer(m_timerId);
        m_timerId = -1;
    }
    if (m_canBusPort)
    {
        m_canBusPort->stopCan();
    }
}
```

Timer de-activation has already been covered in
[Step 1](./step_1.md#application-pre-termination). The `stopCan()` method in
`CanBusKit::CanBusIo`, as the name implies, deactivates the CAN bus port and
inhibits any new events from incoming CAN bus traffic.

### Application Post-Termination

After the application has exited its main run loop, the only thing left to do is
to free up the resources allocated for the CAN bus port. Provide the following
implementation to the `applicationDidTerminate()` method in the
`src/MyAppDelegate.cpp` class:

```c++
void
MyAppDelegate::applicationDidTerminate(Application *theApp)
{
    m_canBusPort.reset(nullptr);
}
```

## Building

The application build tree configuration is done using nearly the same commands
as in previous steps:

```console
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DFoundation_DIR=${FOUNDATION_ROOT}/build/Foundation \
  -G Ninja ..
```

Where `FOUNDATION_ROOT` refers to the environment variable used when cloning
the directory [in the boostrapping section](#bootstrap-the-project-tree).

Once the build environment is configured, the next step is to actually run the
build:

```console
$ ninja
```

## Running

### Pre-Requisites

As discussed in the
[preamble of this tutorial](#tutorial-step-4-io-diversification), this
example application requires a CAN bus port to work. Fortunately, most major
GNU/Linux distributions provide a virtual version of one that is
indistinguishable from application code. The pre-requisite software and kernel
modules were already prepared in that section. What is left is the creation of
the virtual CAN bus port. For that, the networking and traffic control tools
provided by the [`iproute2` suite][iproute2_site]. Creation of the virtual CAN
bus device to use can be done with the `ip` tool as follows:

```console
$ sudo ip link add vcan0 type vcan
$ sudo ip link set vcan0 up
```

The commands create a virtual CAN bus device named `vcan0` and activate it. The
result of those commands is evident by inspecting the output from invoking
`ip link status`. One of the entries should read as follows:

```
9: vcan0: <NOARP,UP,LOWER_UP> mtu 72 qdisc noqueue state UNKNOWN mode DEFAULT group default qlen 1000
    link/can 
```

> [!NOTE]
> The leading number, which stands for the interface index, may not necessarily
> match on every machine. The important content is what follows it.

### Execution

With the virtual CAN bus device up and running, and a terminal window with the
shell session sitting on the build tree, the example application can be
started as follows:

```console
$ ./CanInteract --can-if=vcan0
```

The argument to the `--can-if` interface must be the name given to the virtual
CAN bus device created in the [pre-requisites](#pre-requisites). The example
application will not seem to do anything, but the functionality that emits a CAN
message at timer expiration can be verified via one of the previously-installed
CAN bus tools (in the `can-utils` package). Use a separate terminal session to
run:

```console
$ candump vcan0
```

Where `vcan0` is the name of the virtual CAN bus device. The output should look
something like the following:

```console
  vcan0  100   [4]  00 00 00 2B
  vcan0  100   [4]  00 00 00 2C
  vcan0  100   [4]  00 00 00 2D
  vcan0  100   [4]  00 00 00 2E
  vcan0  100   [4]  00 00 00 2F
  vcan0  100   [4]  00 00 00 30
  vcan0  100   [4]  00 00 00 31
  vcan0  100   [4]  00 00 00 32
```

The output shows the name of the interface (`vcan0`), the CAN ID (`100`), the
data length code (DLC) `[4]`, and the payload bytes.

> [!NOTE]
> The frame payload contents may not match exactly, as the application is 
> emitting an incrementing counter. The format, however, should match.

Terminate the `candump` session with *Ctrl+C* and next test the application's
ability to receive messages using the `cansend` tool:

```console
$ cansend vcan0 200#AABBCCDD
```

Where `vcan0` is the name of the virtual CAN bus device and `200#AABBCCDD` is
the text encoding of a CAN bus frame with ID 0x200, DLC of 4, and a payload with
`[0xAA, 0xBB, 0xCC, 0xDD]`. The result should be a log message from the
application noting receipt of the CAN bus message:

```console
[2024-05-28T19:11:32.510Z] [CanInteract] [446538] [DEBUG]: CAN Input: 0x00000200 [4] AABBCCDD
```

## Conclusion

Congratulations! 🎉🎉 You have created a Foundation application that can interact
with an external device that exposes a CAN bus interface. You exercised
full-duplex communication by both sending and receiving messages from the device
interface. It should be noted that the example application written as-is can
interact over a real CAN bus interface.

[can_bus_versions]: https://en.wikipedia.org/wiki/CAN_bus#Versions "CAN bus versions"
[can_frame_discussion]: https://en.wikipedia.org/wiki/CAN_bus#Frames "CAN bus frame discussion"
[can_utils_pkg]: https://github.com/linux-can/can-utils "SocketCAN Userspace Utilites"
[iproute2_site]: https://wiki.linuxfoundation.org/networking/iproute2 "IPRoute2
