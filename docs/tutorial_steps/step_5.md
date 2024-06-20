# Tutorial Step 5: More I/O Diversity

In [Step 4](./step_4.md), the example application exchanged messages with
external entities over a Controller Area Network (CAN) bus interface. Together
with the [Step 2](./step_2.md) and [Step 3](./step_3.md) examples, it begins to
demonstrate the breadth of applications for the Foundation SDK. Both those
interfaces, however, primarily focus on interfacing with specialized hardware.
The Foundation can also be used to interface with peripherals capable of
communicating over more common network fabric, such as Internet Protocol (IP)
networks.

This example will show how a Foundation application can interact with another
entity, be it a hardware device or a software service, using the User Datagram
Protocol over IP (UDP/IP). The overall design of this application includes:

* A periodic timer that will elicit transmission of a UDP packet containing a
  text message.
* An input event handler that will report on any packets received over UDP.

## Bootstrap the Project Tree

Although this example is notionally very similar to the CAN bus example in
[Step 4](./step_4.md), the mechanics via which they carry out the logic is quite
different. Thus, the best way to bootstrap the project tree is to start from the
baseline skeleton provided by the Foundation source tree, similar to
[Step 1](./step_1.md).  Giving the project folder the name `TutorialStep5`, with
the `FOUNDATION_ROOT` environment variable pointing to the root folder of the
Foundation tree:

```console
$ mkdir TutorialStep5
$ cd TutorialStep5
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
UDP networking. Edit the project metadata in the `CMakeLists.txt` `project()`
command as follows:

```diff
- project(SampleApp LANGUAGES CXX)
+ project(UdpInteract LANGUAGES CXX)
```

Then, update the project dependencies in the `find_package()` command:

```diff
-find_package(Foundation REQUIRED CoreKit)
+find_package(Foundation REQUIRED CoreKit NetworkKit)
```

The `NetworkKit` module in the Foundation SDK brings in the portions required to
support communications over IP, be it UDP or Transmission Control Protocol (TCP;
discussed in another example). Finally, update the dependency list for the
application target in the `target_link_libraries()` command:

```diff
-target_link_libraries(${PROJECT_NAME} Foundation::CoreKit)
+target_link_libraries(${PROJECT_NAME} Foundation::CoreKit Foundation::NetworkKit)
```

## Application Main Function

There are very little changes required of the main function, just like in
[Step 1](./step_1.md). The change involves altering the application name for
logging purposes in the call to `::CoreKit::Application::initialize()` in the
`src/main.cpp` file as follows:

```diff
- G_MyApp->initialize("ApplicationName", argc, argv);
+ G_MyApp->initialize("UdpInteract", argc, argv);
```

## Application Delegate

Again, the vast majority of the changes required to customize the application
lay in the application delegate.

### Application-Specific Logic

The use of UDP/IP networking in the example application requires the addition of
fields that contain the configuration of a UDP communication socket (such as
the socket's IP address and port number). The example as designed also makes use
of a periodic timer, which necessitates implementing the
`::CoreKit::InterruptListener` interface as well as the storing of the timer
identifier.

Begin by altering the definition of the `MyAppDelegate` class in the
`src/MyAppDelegate.h` file towards the beginning as follows:

```diff
#ifndef _MY_APP_DELEGATE_H_
#define _MY_APP_DELEGATE_H_

+#include <memory>
+#include <string>
#include <CoreKit/CoreKit.h>
+#include <NetworkKit/NetworkKit.h>

-class MyAppDelegate : public CoreKit::AppDelegate
+class MyAppDelegate : public CoreKit::AppDelegate, public CoreKit::InterruptListener
{
```

The application delegate class will need a method to handle the timer expiration
events as well as a method to handle incoming UDP packets. Alter the
`MyAppDelegate` class definition body in `src/MyAppDelegate.h` as follows:

```diff
    virtual void applicationDidTerminate(CoreKit::Application *theApp) override;

+   virtual void timerExpired(int timerId) override;
+
+   void onUdpPacket(NetworkKit::UdpPacketNotification const &aNotification);
```

Finally, add the required instance fields to retain the UDP socket
configuration, the timer identifier, and the UDP socket object by altering the
`MyAppDelegate` class body towards the end of `src/MyAppDelegate.h` as follows:

```diff
+private:
+   std::string m_message;
+   std::string m_myIpAddress;
+   int m_myPort;
+   std::unique_ptr< NetworkKit::UdpPacketDistribution > m_packetDistribution;
+   std::string m_peerIpAddress;
+   int m_peerPort;
+   int m_timerId;
+   std::unique_ptr< NetworkKit::UdpSocket > m_udpSocket;
};
```

Up until this example, applications have only had to concern themselves with
their own communication endpoint; there has not been a need to consider location
information regarding the peer. In the serial port example, the connection is
implied to be point-to-point, thus the peer location is implied. In the CAN bus
example, the messages are broadcast to all end items on the bus, and it is up to
the receiving end points to filter the incoming information. In the case of
UDP/IP, packets must be directed at a particular end point. Thus, the
application must store information regarding its own end point *as well as*
information on the peer it communicates with.

> [!NOTE]
> UDP/IP sockets are *not* strictly point-to-point communication ports like
> typical RS-232 or RS-422 serial ports. One application is free to send packets
> to any recipient on the network. In this example, however, in order to
> maintain simplicity, only one peer is supported.

The delegate class also makes room for the actual text message that will be sent
over UDP packets. The utility for such a feature will become clear towards the
end.

The `MyAppDelegate` implementation in `src/MyAppDelegate.cpp` needs to include
the following features:

* Add a command-line argument that lets the application user specify:
  * The IP address and port number the application should bind its UDP socket
    to.
  * The IP address and port number the application should send packets to.
  * The message the application should send over UDP.
* Initialization and termination of the UDP socket.
* Registration of an event handler for incoming UDP packets.
* Registration of a periodic timer.
* Transmission of a UDP packet at every timer expiration.
* Report of any received UDP packets to the application log.

The line items involved with initialization, termination, and event handler
registration will be covered when the changes required in lifecycle methods is
covered. To start off, add the following content towards the beginning of the
`src/MyAppDelegate.cpp` class:

```diff
+#include <algorithm>
+#include <functional>
#include "MyAppDelegate.h"

+using CoreKit::Application;
+using CoreKit::AppLog;
+using CoreKit::EndLog;
+using CoreKit::G_MyApp;
+using NetworkKit::UdpIpPacketNotification;
+using NetworkKit::UdpPacketDistribution;
+using NetworkKit::UdpPacketNotification;
+using NetworkKit::UdpSocket;

+namespace
+{
+   const char *MESSAGE_FLAG = "message";
+   const char *MY_IP_FLAG = "my-ip";
+   const char *MY_PORT_FLAG = "my-port";
+   const char *PEER_IP_FLAG = "peer-ip";
+   const char *PEER_PORT_FLAG = "peer-port";
+}
+
-MyAppDelegate::MyAppDelegate()
+MyAppDelegate::MyAppDelegate():
+   m_myPort(-1),
+   m_peerPort(-1),
+   m_timerId(-1)
{

}
```

Like that first introduced in [Step 4](./step_4.md#application-specific-logic),
this example also makes use of namespace imports for scoped names and private
constants for command-line flag names.

Next, provide the implementation of the `configureCmdLineArgs()` method in the
`src/MyAppDelegate.cpp` file to add support for all the command-line flags:

```c++
void
MyAppDelegate::configureCmdLineArgs(Application *theApp)
{
    theApp->addCmdLineArgDef(
        Application::CmdLineArg(
            MESSAGE_FLAG,
            true,
            "Short text message to send to the peer."
        )
    );
    theApp->addCmdLineArgDef(
        Application::CmdLineArg(
            MY_IP_FLAG,
            true,
            "IP address that our UDP socket should bind to."
        )
    );
    theApp->addCmdLineArgDef(
        Application::CmdLineArg(
            MY_PORT_FLAG,
            true,
            "UDP port number that our UDP socket should bind to."
        )
    );
    theApp->addCmdLineArgDef(
        Application::CmdLineArg(
            PEER_IP_FLAG,
            true,
            "Destination IP address for packets."
        )
    );
    theApp->addCmdLineArgDef(
        Application::CmdLineArg(
            PEER_PORT_FLAG,
            true,
            "Destination UDP port for packets."
        )
    );
}
```

After that, add the logic that will emit a UDP packet at every timer expiration
in the implementation of the `timerExpired()` method in the
`src/MyAppDelegate.cpp` class:

```c++
void
MyAppDelegate::timerExpired(int timerId)
{
    m_udpSocket->sendTo(m_peerIpAddress, m_peerPort, m_message);
}
```

The `sendTo()` method in the `::NetworkKit::UdpSocket` class is pretty self
explanatory in its use. The first two (2) arguments are the location where the
packet will be sent (destination IP address and port number), and the third
argument is the packet contents to send. In this example, the packet contents is
simply the message configured via the command line.

With the periodic emitting logic in place, add the logic that will receive and
report on any incoming CAN bus traffic by providing the following
`onUdpPacket()` implementation in the `src/MyAppDelegate.cpp` file:

```c++
void
MyAppDelegate::onUdpPacket(UdpPacketNotification const &aNotification)
{
    std::string receivedText;

    // downcast to UDP/IP packet
    auto *udpIpNotification =
        dynamic_cast<UdpIpPacketNotification const *>(&aNotification);
    if (nullptr != udpIpNotification)
    {
        receivedText.resize(udpIpNotification->packetContents.size());
        std::copy(
            udpIpNotification->packetContents.begin(),
            udpIpNotification->packetContents.end(),
            receivedText.begin()
        );

        G_MyApp->log() << AppLog::LL_INFO
            << "Received: "
            << receivedText
            << " from "
            << udpIpNotification->ipAddress
            << ":"
            << udpIpNotification->port
            << EndLog;
    }
}
```

The implementation of `onUdpPacket()` shows towards the beginning that the
`::NetworkKit::UdpPacketNotification` class is the base class of a hierarchy
designed to support two (2) distinct types of UDP sockets: those based on IP,
and those based on the UNIX file system. The `dynamic_cast<>` operator is used
to safely downcast to the type of notification emitted for IP-based sockets. The
majority of fields is shared between the different notification sub-classes.
They only differ in the source address information they contain. The `ipAddress`
and `port` fields are exclusive to the IP-based notification class, whereas the
`packetContents` field is common across all.

Similar to the implementation of the `onSerialInput()` method shown in
[Step 3](./step_3.md#application-specific-logic), the implementation of
`onUdpPacket()` performs a very rudimentary form of string decoding by using the
C++11 `std::copy()` function to copy the contents of the packet onto a
`std::string` instance.

### Application Initialization

During initialization this example application must acquire all of the required
configuration from the command-line via the configured flags. The application
must also allocate the UDP socket to use, as well as the packet distribution hub
that will disseminate incoming packets to event handlers. Add the following
implementation to the `applicationDidInitialize()` method in the
`src/MyAppDelegate.cpp` class:

```c++
void
MyAppDelegate::applicationDidInitialize(Application *theApp)
{
    m_message = theApp->getCmdLineArgFor(MESSAGE_FLAG);
    if (m_message.empty())
    {
        throw std::runtime_error("No message to send provided.");
    }
    m_myIpAddress = theApp->getCmdLineArgFor(MY_IP_FLAG);
    if (m_myIpAddress.empty())
    {
        throw std::runtime_error("No IP address to bind provided.");
    }
    m_myPort = std::stoi(theApp->getCmdLineArgFor(MY_PORT_FLAG));
    m_peerIpAddress = theApp->getCmdLineArgFor(PEER_IP_FLAG);
    if (m_peerIpAddress.empty())
    {
        throw std::runtime_error("No destination IP address provided.");
    }
    m_peerPort = std::stoi(theApp->getCmdLineArgFor(PEER_PORT_FLAG));
    m_udpSocket.reset(new UdpSocket(m_myIpAddress, m_myPort));
    m_packetDistribution.reset(new UdpPacketDistribution());
}
```

Why is there a need for a "packet distribution" entity, and why wasn't a similar
entity found in the serial port and CAN bus examples? The reason for this stems
from the diverse nature of UDP sockets, particularly the multiple underlying
network protocol layers (per the [OSI model][osi_model]) they support: IP and
UNIX file system. During design of this network kit feature there was a need to
simultaneously support incoming traffic from both varieties. Such a feature
could have been left for application developers to implement, but it was common
enough in applications that the decision to factor out input event distribution
from the input medium was made. When it comes to CAN bus or serial ports, no
such diversity in underlying [OSI model][osi_model] layers exists.

### Application Start

Just before turning over control to the main run loop, the application must
configure and activate the following:

* The UDP socket used for communication.
* The UDP input event handler.
* The periodic timer used to emit packets over UDP.

Provide the implementation of the `applicationStarting()` method in the
`src/MyAppDelegate.cpp` file with the following:

```c++
void
MyAppDelegate::applicationStarting(Application *theApp)
{
    m_timerId = theApp->getMainRunLoop()->registerTimerWithInterval(1.0, this, true);
    m_packetDistribution->addNotificationCallback(
        std::bind(
            std::mem_fn(&MyAppDelegate::onUdpPacket),
            this,
            std::placeholders::_1
        )
    );
    m_udpSocket->initialize(m_packetDistribution.get());
    theApp->getMainRunLoop()->registerInputSource(m_udpSocket.get());
}
```

The registration of a member function as an event handler, particularly the use
of the `std::bind()` and `std::mem_fn()` C++11 calls, is covered in the
[Application Start discussion of Step 3](./step_3.md#application-start). Unlike
other examples up to this point, the event handler is not directly registered
with the I/O end point but rather with the instance of
`::NetworkKit::UdpPacketDistribution`. That instance is provided to the UDP
socket via its `initialize()` method. The socket will only notify the packet
distribution entity whenever a UDP packet arrives, and it is up to the
distribution entity to broadcast receipt to the registered event handlers. The
UDP socket must also be directly registered as an input source with the
application's main run loop via the `registerInputSource()` method. This is
something that happens at lower layers in the other examples, such as the call
to `startCan()` in [Step 4](./step_4.md#application-start) or `startSerial()` in
[Step 3](./step_3.md#application-start).


### Application Pre-Termination

Orderly shutdown of the application involves deactivating any services driving
the main run loop. For this application, that involves stopping the periodic
timer and de-registering the UDP socket as an input source. Add the
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
    if (m_udpSocket)
    {
        theApp->getMainRunLoop()->deregisterInputSource(m_udpSocket.get());
        m_udpSocket->terminate();
    }
}
```

Deactivation of the UDP socket is a two-step process: the socket must be removed
from the list of input sources in the main run loop, and the socket itself must
be terminated. With the socket terminated, the packet distribution entity is
safe to remain lingering, as no events are incoming.

### Application Post-Termination

After the application has exited its main run loop, the only thing left to do is
to free up the resources allocated for the socket and the packet distribution
entity. Provide the following implementation to the `applicationDidTerminate()`
method in the `src/MyAppDelegate.cpp` class:

```c++
void
MyAppDelegate::applicationDidTerminate(Application *theApp)
{
    m_packetDistribution.reset(nullptr);
    m_udpSocket.reset(nullptr);
}
```

Destruction of the packet distribution entity automatically de-registers the
input event handlers registered in the [application start](#application-start)
section.

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

Up until this example, we have needed an external tool to test the logic of the
application, whether that was `socat` or `can-utils`. In this example, two (2)
instances of the application running simultaneously can serve as mutual test
targets, so as long as each instance is distinguishable. This is where
configuration of the message to emit via the command line comes in handy. Open
two (2) terminal sessions with the shell at the application build tree, and run
an instance of the application in one as follows:

```console
$ ./UdpInteract --message="One" --my-ip=127.0.0.1 --my-port=7000 --peer-ip=127.0.0.1 --peer-port=7001
```

On the other terminal session window, start another instance of the application
as follows:

```console
$ ./UdpInteract --message="Two" --my-ip=127.0.0.1 --my-port=7001 --peer-ip=127.0.0.1 --peer-port=7000
```

Notice the complement that both instances make, where the self address of one is
the peer address of the other. It should be noted that even though this
particular setup is meant to run on a single host, it is very much possible to
alter the command-line parameters to have the two instances communicate over a
network.

Output from the first terminal window should reflect the traffic emitted by the
second copy:

```
[2024-05-28T22:08:42.968Z] [UdpInteract] [452745] [INFORMATION]: Received: Two from 127.0.0.1:7001
[2024-05-28T22:08:43.968Z] [UdpInteract] [452745] [INFORMATION]: Received: Two from 127.0.0.1:7001
[2024-05-28T22:08:44.968Z] [UdpInteract] [452745] [INFORMATION]: Received: Two from 127.0.0.1:7001
[2024-05-28T22:08:45.968Z] [UdpInteract] [452745] [INFORMATION]: Received: Two from 127.0.0.1:7001
[2024-05-28T22:08:46.968Z] [UdpInteract] [452745] [INFORMATION]: Received: Two from 127.0.0.1:7001
[2024-05-28T22:08:47.968Z] [UdpInteract] [452745] [INFORMATION]: Received: Two from 127.0.0.1:7001
[2024-05-28T22:08:48.968Z] [UdpInteract] [452745] [INFORMATION]: Received: Two from 127.0.0.1:7001
[2024-05-28T22:08:49.967Z] [UdpInteract] [452745] [INFORMATION]: Received: Two from 127.0.0.1:7001
[2024-05-28T22:08:50.968Z] [UdpInteract] [452745] [INFORMATION]: Received: Two from 127.0.0.1:7001
[2024-05-28T22:08:51.968Z] [UdpInteract] [452745] [INFORMATION]: Received: Two from 127.0.0.1:7001
[2024-05-28T22:08:52.968Z] [UdpInteract] [452745] [INFORMATION]: Received: Two from 127.0.0.1:7001
[2024-05-28T22:08:53.968Z] [UdpInteract] [452745] [INFORMATION]: Received: Two from 127.0.0.1:7001
```

Whereas output from the second terminal window should reflect the traffic
emitted by the first copy:

```
[2024-05-28T22:08:42.040Z] [UdpInteract] [452747] [INFORMATION]: Received: One from 127.0.0.1:7000
[2024-05-28T22:08:43.039Z] [UdpInteract] [452747] [INFORMATION]: Received: One from 127.0.0.1:7000
[2024-05-28T22:08:44.039Z] [UdpInteract] [452747] [INFORMATION]: Received: One from 127.0.0.1:7000
[2024-05-28T22:08:45.039Z] [UdpInteract] [452747] [INFORMATION]: Received: One from 127.0.0.1:7000
[2024-05-28T22:08:46.039Z] [UdpInteract] [452747] [INFORMATION]: Received: One from 127.0.0.1:7000
[2024-05-28T22:08:47.039Z] [UdpInteract] [452747] [INFORMATION]: Received: One from 127.0.0.1:7000
[2024-05-28T22:08:48.039Z] [UdpInteract] [452747] [INFORMATION]: Received: One from 127.0.0.1:7000
[2024-05-28T22:08:49.039Z] [UdpInteract] [452747] [INFORMATION]: Received: One from 127.0.0.1:7000
[2024-05-28T22:08:50.039Z] [UdpInteract] [452747] [INFORMATION]: Received: One from 127.0.0.1:7000
[2024-05-28T22:08:51.039Z] [UdpInteract] [452747] [INFORMATION]: Received: One from 127.0.0.1:7000
[2024-05-28T22:08:52.039Z] [UdpInteract] [452747] [INFORMATION]: Received: One from 127.0.0.1:7000
[2024-05-28T22:08:53.039Z] [UdpInteract] [452747] [INFORMATION]: Received: One from 127.0.0.1:7000
[2024-05-28T22:08:54.039Z] [UdpInteract] [452747] [INFORMATION]: Received: One from 127.0.0.1:7000
```

## Conclusion

Congratulations! 🎉🎉 You have created a Foundation application that communicates
over an IP-based network using UDP datagrams. What's more, you've completed the
first integration of two (2) Foundation applications that communicate with each
other, even if the applications are based off of the exact same code.

[osi_model]: https://en.wikipedia.org/wiki/OSI_model "OSI Model"
