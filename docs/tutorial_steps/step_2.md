# Tutorial Step 2: Reaching Out Over Serial I/O

This tutorial step will evolve the work introduced in [Step 1](./step_1.md) by
emitting the "Hello!" message over a serial port in addition to the application
log on screen. In order to alleviate the need for actual hardware, this example
will make use of the [Linux Pseudo-TTY][Linux_Pseudo_TTY] facilities to create
a virtual serial port. The virtual port will be practically indistinguishable
from the real thing to the Foundation library.

> [!TIP]
> Should a real serial port be available along with methods to verify outgoing
> traffic on it, like a device on the other end that can report what it
> receives, it can be used in place of the pseudo-tty documented here. Care
> should be taken so that the example application runs with enough privileges
> to interact with the serial port.

## Bootstrap the Project Tree

Although, similar to [Step 1](./step_1.md), we could bootstrap the tutorial
project from the skeleton tree provided by the Foundation source code, this
project only plans to augment the functionality in the aforementioned project.
Thus, bootstrapping from the code in [Step 1](./step_1.md) is more efficient.
Giving the project root folder the name `TutorialStep2` and assuming the folder
will be a peer to the project folder created in [Step 1](./step_1.md), invoke
the following statements in the command line:

```console
$ mkdir TutorialStep2
$ cd TutorialStep2
$ rsync -auv --exclude 'build/' --exclude 'compile_commands.json' ../TutorialStep1/ .
```

The portions of the code copied over that will be customized for this tutorial
step are:

* The project build configuration file `CMakeLists.txt`
* The application main function
* The application delegate

## Project Build Configuration

In order to distinguish this project from others, it should be given a unique
name. In addition, the project carries a new dependency on a Foundation module:
that which enables serial port communications. In the `CMakeLists.txt` file,
edit the project metadata in the `project()` CMake command so it bears the
project name:

```diff
- project(InsistentHello LANGUAGES CXX)
+ project(SerialSayHello LANGUAGES CXX)
```

Second, amend the project dependencies to Foundation in the `find_package()`
CMake command to add a dependency on the serial communications module:

```diff
- find_package(Foundation REQUIRED CoreKit)
+ find_package(Foundation REQUIRED CoreKit SerialKit)
```

`SerialKit` is the name of the Foundation SDK module that brings with it serial
port communication facilities. In addition to declaring a dependency on it, the
module must be linked to the executable target. Alter the target dependency
command `target_link_libraries()` so it includes the serial communications
module:

```diff
- target_link_libraries(${PROJECT_NAME} Foundation::CoreKit)
+ target_link_libraries(${PROJECT_NAME} Foundation::CoreKit Foundation::SerialKit)
```

That should be all the changes required to enable the use of serial ports in the
application.

## Application Main Function

There are very little changes required of the main function, just like in
[Step 1](./step_1.md). The change involves altering the application name for
logging purposes in the call to `::CoreKit::Application::initialize()` in the
`src/main.cpp` file as follows:

```diff
- G_MyApp->initialize("InsistentHello", argc, argv);
+ G_MyApp->initialize("SerialSayHello", argc, argv);
```

## Application Delegate

The majority of changes introduced in this tutorial example are concentrated in
the implementation of the application delegate class. There will be changes
required to the application-specific logic, as well as specific points in the
application lifecycle where `::CoreKit::Application` defers to the delegate.

### Application-Specific Logic

The example application must maintain a reference to the serial port it intends
to communicate over. Thus, there is a need to add another private field to the
`MyAppDelegate` class. Addition of the field also precipitates the need for
extra `#include` statements to bring in the necessary definitions. Alter the
`src/MyAppDelegate.h` file towards the beginning to import the `SerialKit`
as well as other standard C++ definitions:

```diff
#ifndef _MY_APP_DELEGATE_H_
#define _MY_APP_DELEGATE_H_

+ #include <memory>
+ #include <string>
#include <CoreKit/CoreKit.h>
+ #include <SerialKit/SerialKit.h>
```

Furthermore, alter the actual `MyAppDelegate` class definition in
`src/MyAppDelegate.h` to add the instance field that will hold a reference to
the serial port, as well as the name of the serial port to use (more on how
this last field is populated later).

```diff

private:
    int m_timerId;
+   std::unique_ptr< SerialKit::SerialIo > m_serialPort;
+   std::string m_serialPortName;
};
```

The class implementation at `src/MyAppDelegate.cpp` must also be altered in
several places:

* The timer expiration handler must emit a message over the serial port in
  addition to the application log on screen.
* The application delegate must configure a command-line argument that lets the
  application user specify what serial port to use.

Alter the beginning of `src/MyAppDelegate.cpp` to import the required
definitions:

```diff
+#include <stdexcept>
+#include <string>
#include "MyAppDelegate.h"
```

Next, alter the implementation of the `configureCmdLineArgs()` method to add
a new command-line argument definition that gives the application user the
ability to specify the name of the serial port with a `--serial-port` flag:

```diff
void
MyAppDelegate::configureCmdLineArgs(Application *theApp)
{
+   theApp->addCmdLineArgDef(
+       Application::CmdLineArg(
+           "serial-port",
+           true,
+           "Serial port (e.g., /dev/tty?) to use"
+       )
+   );
}
```

The `addCmdLineArgDef()` method in the `::CoreKit::Application` class configures
the command-line parser built in to Foundation. The method accepts as its lone
argument a just-in-time constructed instance of the
`::CoreKit::Application::CmdLineArg` class. That class constructor accepts three
(3) arguments:

1. The literal name of the command-line flag users should use, except for the
  leading `--`.
1. Whether the command-line flag needs to be accompanied by an argument (`true`)
  or not (`false`).
1. A short description of the flag that can be shown to users.

The last argument to `CmdLineArg` is used to create a help message available to
users when they invoke the application with the `--help` command-line flag.
Interpreting the `--help` flag and serving the help message is another example
of a built-in Foundation SDK feature. It is even present in the example
application created in [Step 1](./step_1.md). Running that application with
`--help` will show it in action.

> [!NOTE]
> The call to `configureCmdLineArgs()` in the application delegate is not part
> of the standard `::CoreKit::Application` lifecycle events, but it has been
> present in every application main function body. Browse through the
> `src/main.cpp` file to confirm.

Then, alter the `timerExpired()` method so that it sends the message over the
serial port in addition to sending a log message:

```diff
void
MyAppDelegate::timerExpired(int timerId)
{
+   std::string message("Hello!");
+
    CoreKit::G_MyApp->log() << CoreKit::AppLog::LL_INFO << message << CoreKit::EndLog;
+   m_serialPort->sendData(message);
}
```

> [!NOTE]
> A more robust application logic would enforce the pre-requisite that the
> `m_serialPort` be initialized prior to use. For the sake of brevity, such
> checks are omitted here.

The `sendData()` method in the
[`::SerialKit::SerialIo` class](../../SerialKit/SerialIo.h) accepts any C++
container that provides an object satisfying the
[C++ *LegacyIterator* Named Requirement][Cpp_Legacy_Iter] via its `begin()` and
`end()` methods, such as `std::string`, `std::vector`, and several custom
containers provided by the Foundation.

### Application Initialization

Unlike the application in [Step 1](./step_1.md) that did not have any need to
perform tasks immediately after initialization, this application does:

* This application must process the serial port name provided by the user via
  the `--serial-port` command-line flag.
* It must also create an instance of the serial port that will be used at every
  timer expiration.

Alter the implementation of the `applicationDidInitialize()` method in
`src/MyAppDelegate.cpp` file as follows:

```diff
void
MyAppDelegate::applicationDidInitialize(Application *theApp)
{
+   m_serialPortName = theApp->getCmdLineArgFor("serial-port");
+   if (m_serialPortName.empty())
+   {
+      throw std::runtime_error("No serial port name provided.");
+   }
+
+   m_serialPort.reset(
+       new SerialKit::SerialIo(
+           m_serialPortName,
+           theApp->getMainRunLoop(),
+           &theApp->log()
+       )
+   );
}
```

The `getCmdLineArgFor()` method in `::CoreKit::Application` attempts to fetch
the argument provided by the user via the command-line flag identified in the
method arguments (`serial-port` in this case). If the command-line flag was not
present when the application was invoked, the method returns an empty string.
Hence the check to ensure the serial port name was specified, and the exception
abort if it was not.

The `::SerialKit::SerialIo` class represents the serial port to use. The
constructor accepts three (3) arguments:

1. The name of the serial port. In this case, the name is as acquired from the
  command-line arguments.
1. A pointer to the run loop that will monitor the serial port. In this case,
  the application main run loop will monitor the serial port.
1. A pointer to the application log object.

> [!NOTE]
> A more robust application logic would enforce the pre-requisite that the
> `m_serialPort` field not be initialized prior to the call to
> `applicationDidInitialize()`, but for the sake of brevity it is omitted here.

### Application Start

This example application has the same start-up requirements as that of
[Step 1](./step_1.md), the creation of the periodic timer, but it has another
one: the activation of the serial port. Amend the implementation of the
`applicationStarting()` method as follows:

```diff
void
MyAppDelegate::applicationStarting(Application *theApp)
{
    m_timerId = theApp->getMainRunLoop()->registerTimerWithInterval(1.0, this, true);
+   m_serialPort->startSerialIo(B115200, true);
}
```

The `startSerialIo()` method in the `::SerialKit::SerialIo` class can either be
called with no arguments, or with two (2) arguments as done in this example.
When called with no arguments, the class makes no attempt to configure the
serial port prior to use. The class assumes the port has already been configured
and uses it as-is. When called with two (2) arguments, the class configures the
port with the baud rate as specified in the first argument (115,200 baud in
this case) and either in "raw" or "processed" mode based on the second argument
("raw" in this case). The constants used to configure the baud rate are
documented in the [termios man page][termios_man_page].

The segmented initialization/activation of the serial port between the
[application initialization](#application-initialization) and
[application start](#application-start) lifecycle phases may seem odd, but that
exists as so because the serial port is a possible source of stimuli for the
application. Thus, activation must only happen just before the application is
ready to accept external stimuli. This example application ignores stimuli from
the serial port, but aims to follow the initialization/activation pattern.

### Application Pre-Termination

This example application has the same pre-termination requirement as that from
[Step 1](./step_1.md), plus it must de-activate the serial port stimuli source.
Alter the implementation of the `applicationWillTerminate()` method as follows:

```diff
void
MyAppDelegate::applicationWillTerminate(Application *theApp)
{
    if (m_timerId != -1)
    {
        theApp->getMainRunLoop()->deregisterTimer(m_timerId);
        m_timerId = -1;
    }
+   if (m_serialPort)
+   {
+       m_serialPort->stopSerialIo();
+   }
}
```

The `stopSerialIo()` method in `::SerialKit::SerialIo`, as the name implies,
deactivates the serial port, inhibiting it from producing any stimuli.

### Application Post-Termination

This example application allocated resources for the serial port at the
[Application Initialization](#application-initialization) lifecycle step, thus
it must release those resources once the application's main loop has terminated.
Add the following implementation to the `applicationDidTerminate()` method in
`src/MyAppDelegate.cpp`:

```diff
void
MyAppDelegate::applicationDidTerminate(Application *theApp)
{
+   m_serialPort.reset(nullptr);
}
```

## Building

Just like in the [Step 1 Building section](./step_1.md#building), the
application build is split into two phases: build environment configuration and
build execution. Configure the build environment in a separate build tree as
follows:

```console
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DFoundation_DIR=<Foundation Build Tree>/Foundation \
  -G Ninja ..
```

Where `<Foundation Build Tree>` refers to the directory that contains the build
tree for the Foundation, as created after following the
[Foundation build instructions](../building.md).

Once the build environment is configured, the next step is to actually run the
build:

```console
$ ninja
```

## Running

### Pre-requisites

As discussed in the
[preamble of this tutorial](#tutorial-step-2-reaching-out-over-serial-io), this
example application requires a serial port to work. Thus, the plan is to create
a pseudo-TTY to act as the serial port. The [`socat` tool][socat_home] is an
easy choice to create a pseudo terminal that our application can interact with.
The tool is available not just from the linked home page, but from just about
all major Linux distribution package managers. As an example, installing for
Debian/Ubuntu would be:

```console
$ sudo apt install socat
```

Once `socat` is available, open a terminal that can remain open as a feedback
window, and run the following from the build tree:

```console
$ socat PTY,link=$PWD/pty,echo=0 STDIO
```

The command will not seem to do much, but it has created a pseudo-TTY, created
a link to it from the build tree and called it `pty`, and stands ready to echo
anything that is sent over the port.

### Execution

Open another terminal, navigate to the build tree, and run the application as
follows:

```console
$ ./SerialSayHello --serial-port=$PWD/pty
```

Similar to the application in [Step 1](./step_1#running), it will output log
messages every time it transmits over the serial port. The window left open for
`socat` will also report the serial port activity:

```
$ socat PTY,link=$PWD/pty,echo=0 STDIO
Hello!Hello!Hello!Hello!Hello!Hello!Hello!Hello!Hello!
```

> [!WARNING]
> The example application as written is not very robust, and having the serial
> port "disappear" by closing down the `socat` session before the application
> will result in a flurry of error messages.

## Conclusion

Congratulations! 🎉🎉 You have successfully integrated a serial port, albeit a
virtual one, to a Foundation application. This establishes the first step
in writing applications that interface with external devices. Although the
messages exchanged are plain text, which is unlikely to be the case when
communicating with a real world device, the fundamentals and mechanics are still
the same would this example be exchanging fixed or variable sized frames with a
target.

[Cpp_Legacy_Iter]: https://en.cppreference.com/w/cpp/named_req/Iterator "C++ LegacyIterator Named Requirement"
[Linux_Pseudo_TTY]: https://man7.org/linux/man-pages/man7/pty.7.html "Linux Pseudo-TTY"
[socat_home]: http://www.dest-unreach.org/socat/ "socat Home Page"
[termios_man_page]: https://man7.org/linux/man-pages/man3/termios.3.html "termios Man Page"
