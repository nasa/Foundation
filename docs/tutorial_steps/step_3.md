# Tutorial Step 3: Listening and Reacting

[Step 2](./step_2.md) of the Foundation tutorial demonstrated how to emit
messages through an external interface to, potentially, a target device. In this
example, that application will be modified such that it will not automatically
emit a message based off a periodic timer, but instead monitor incoming messages
and acknowledge receipt back to the sender.

> [!NOTE]
> This example, as the previous, will rely on a virtual serial port based on a
> Linux Pseudo-TTY.

## Bootstrap the Project Tree

The code from [Step 2](./step_2.md) is an excellent starting point for this
example, therefore its contents will be used to bootstrap this example project.
Giving the project root folder the name `TutorialStep3` and assuming the folder
will be a peer to the project folder from [Step 2](./step_2.md), execute the
following statements in the command line:

```console
$ mkdir TutorialStep3
$ cd TutorialStep3
$ rsync -auv --exclude 'build/' --exclude 'compile_commands.json' ../TutorialStep2/ .
```

The portions of the code copied over that will be customized for this tutorial
step are:

* The project build configuration file `CMakeLists.txt`
* The application main function
* The application delegate

## Project Build Configuration

The dependencies listed in the project build configuration for
[Step 2](./step_2.md#project-build-configuration) are the same for this project,
thus the only customization required is the renaming of the project.In the
`CMakeLists.txt` file, edit the project metadata in the `project()` CMake
command so it bears the project name:

```diff
- project(SerialSayHello LANGUAGES CXX)
+ project(SerialCopyBack LANGUAGES CXX)
```

## Application Main Function

There are very little changes required of the main function, just like in
[Step 2](./step_2.md#application-main-function). The only required change
involves altering the application name for logging purposes when calling
`::CoreKit::Application::initialize()` in the `src/main.cpp` file as follows:

```diff
- G_MyApp->initialize("SerialSayHello", argc, argv);
+ G_MyApp->initialize("SerialCopyBack", argc, argv);
```

## Application Delegate

A common theme has started to emerge along these tutorial steps: the majority
of changes to an application focus on the delegate class. This pattern holds
true even in production-level Foundation applications. In production, another
area that sees a lot of change is the project build configuration, but the
relative simplicity of these examples has limited such impacts. For this
example application, the areas in the application delegate that require changes
are: the application custom logic and the lifecycle point implementations.

### Application-Specific Logic

This example application retains the use of the serial port as that of
[Step 2](./step_2.md), but it no longer needs the periodic timer to emit
messages. Instead, it will only emit upon receipt of a message. The message
receipt event will be handled in a method similar to how timer expiration events
are handled. Modify the `MyAppDelegate` class definition in the
`src/MyAppDelegate.h` file to remove the timer ID instance field, since it is
no longer needed, and rename the `timerExpired()` method to something more apt
like `onSerialInput()`.

```diff
    virtual void applicationDidTerminate(CoreKit::Application *theApp) override;

-   virtual void timerExpired(int timerId) override;
+   void onSerialInput(SerialKit::SerialDataNotification *dataNotif);

private:
-   int m_timerId;
    std::unique_ptr< SerialKit::SerialIo > m_serialPort;
```

There are several points that merit discussion along with these changes. The
first would be the name of the handler. When handling timers, the target class
is bound by two (2) requirements:

* Implement the `::CoreKit::InterruptListener` interface.
* Provide an implementation for the `timerExpired()` method in said interface.

When it comes to handling input from external devices, however, there is but
just one requirement:

* Provide a callable entity with the appropriate call interface.

The discrepancy arises from the relative level between those two event handlers.
The timer event handler is a lower level, more fundamental entity, whereas the
external input handler is higher level. In fact, code within the Foundation
makes use of the lower-level API to implement the application-facing API for
external input. Given that such input is by and large going to be handled
similarly, it made sense to add that layer of abstraction.

Going back to the external input handler requirement statement, particularly the
portion mentioning the *"...appropriate call interface..."*, it becomes clear
that for serial port I/O, the input handling callable must accept a single
argument of type [`::SerialKit::SerialDataNotification`](../../SerialKit/SerialDataNotification.h).
The callable that handles the input need not be a member function to the
application delegate class (or to any class, for that matter), but it does
simplify implementation in this example.

The implementation of the `MyAppDelegate` class must now be modified to match
the class declaration changes. There will also be a need to import other C++
standard features used later. First, add the required `#include` statements in
the `src/MyAppDelegate.cpp` file to import such features:

```diff
+#include <algorithm>
+#include <functional>
#include <stdexcept>
#include <string>
```

Then, alter the class constructor implementation by removing the initialization
of the `m_timerId` instance field:

```diff
-MyAppDelegate::MyAppDelegate():
-   m_timerId(-1)
+MyAppDelegate::MyAppDelegate()
{

}
```

In addition, the `timerExpired()` method implementation must be swapped out with
the implementation of the new `onSerialInput()` method in the class declaration.
After removing the `timerExpired()` implementation, add this for
`onSerialInput()`:

```c++
void
MyAppDelegate::onSerialInput(SerialKit::SerialDataNotification *dataNotif)
{
    std::string receivedText;
    std::string response;

    receivedText.resize(dataNotif->serialData.size());
    std::copy(dataNotif->serialData.begin(), dataNotif->serialData.end(), receivedText.begin());
    CoreKit::G_MyApp->log() << CoreKit::AppLog::LL_DEBUG
                   << "Received \""
                   << receivedText
                   << "\" from the serial port at \""
                   << dataNotif->serialPort
                   << "\""
                   << CoreKit::EndLog;
    response = std::string("Received \"") + receivedText + "\"\n";
    m_serialPort->sendData(response);
}
```

There are several points that also bear mention here, mostly dealing with the
`::SerialKit::SerialDataNotification` class and the instance provided to the
event handler. First to discuss is the kind of information that is included in
instances of that class:

* `serialPort` field - The name of the serial port where the data arrived.
* `serialData` field - `std::vector`-like container with the bytes received
  through the serial port.
* `acqTime` field (not shown in example) - `struct timespec` instance
  pinpointing the date and time at which the data arrived.

The `serialPort` and `acqTime` fields are pretty self-explanatory. The
`serialData` field is also easy to comprehend, but there are a few points to
keep in mind:

* The field contains the bytes that were available for reading at the serial
  port without blocking the calling thread.
* The field makes no attempt to encode/decode the incoming data (such as
  encoding to text).

The second point regarding encoding is why there was a need to use the C++
[`std::copy()` algorithm facility][Cpp_copy] to copy the incoming raw bytes into
a `std::string` object. No actual decoding was done (such as to/from UTF-8), but
this example only deals with printable ASCII so there was no need for it.

The first point is most important, as the input received may not necessarily
constitute a complete, coherent message to the receiver. Given that serial ports
deal in streams of bytes, it is very possible (in fact, quite likely) that a
single read will not return an entire, coherent message (such as, say, a text
sentence or a complete data frame). Application code must be prepared to handle
such fragmentation.

> [!WARNING]
> In an attempt to handle such fragmentation, developers may be tempted to
> preserve the `::SerialKit::SerialDataNotification` instance provided to the
> input handler, but that would be a mistake: the lifetime of the aforementioned
> object instance is only guaranteed while the input event callback is in scope.
> A more appropriate way would be to copy the data communicated via the
> notification object to a temporary buffer for further processing.

### Application Initialization

Compared against the
[application initialization of Step 2](./step_2.md#application-initialization),
this example application does not have any different needs during the same
lifecycle step, so no changes are required in the `applicationDidInitialize()`
method implementation.

### Application Start

Since this example application did away with the periodic timer, the logic that
initialized it in the `applicationStarting()` method must be removed. In
addition, an event handler for serial port input must be registered. Alter the
implementation of `applicationStarting()` in `src/MyAppDelegate.cpp` as follows:

```diff
void
MyAppDelegate::applicationStarting(Application *theApp)
{
-   m_timerId = theApp->getMainRunLoop()->registerTimerWithInterval(1.0, this, true);
+   m_serialPort->registerSerialDataCallback(
+       SerialKit::newSerialDataCallback(
+           std::bind(
+               std::mem_fn(&MyAppDelegate::onSerialInput),
+               this,
+               std::placeholders::_1
+           )
+       )
+   );
    m_serialPort->startSerialIo(B115200, true);
}
```

The `registerSerialDataCallback()` method in the `::SerialKit::SerialIo` is
used to register a handler that will be called anytime input arrives at the
serial port. The method accepts an object instance derived from the
`::SerialKit::SerialDataCallback` class. The convenience
`::SerialKit::newSerialDataCallback()` function serves as a factory for these
callback objects. The function accepts as its lone argument a callable that
should accept a `::SerialKit::SerialDataNotification` pointer as its only
argument. A pointer to `MyAppDelegate::onSerialInput()`, being a member function,
is not directly callable without the object instance it is to be called against.
Thus, the [C++ function object][Cpp_Func_Objs] utility
[`std::mem_fn()`][Cpp_mem_fn] is used to create a callable that bears the same
call interface as the provided instance method while prepending the object
instance as the first argument. In order to match the required call interface
for the event handler, that just-in-time-created callable is transformed so that
its first argument is bound to the `MyAppDelegate` object instance using
[`std::bind()`][Cpp_bind], while leaving the second argument, the pointer to
`::SerialKit::SerialDataNotification`, as a "placeholder."

### Application Pre-Termination

Again, since this example application did away with the periodic timer, the
logic that stops it prior to termination must also be removed. Alter the
implementation of `applicationWillTerminate()` in `src/MyAppDelegate.cpp` as
follows:

```diff
void
MyAppDelegate::applicationWillTerminate(Application *theApp)
{
-   if (m_timerId != -1)
-   {
-       theApp->getMainRunLoop()->deregisterTimer(m_timerId);
-       m_timerId = -1;
-   }
    if (m_serialPort)
    {
        m_serialPort->stopSerialIo();
    }
}
```

Explicit de-registration of the input handler registered during the
[application start](#application-start) lifecycle event is not required, as all
input handler registrations are purged automatically when the `stopSerialIo()`
in `::SerialKit::SerialIo` is called.

### Application Post-Termination

No differences exist in the required post-termination logic between this example
and that of [Step 2](./step_2.md#application-post-termination), so no changes
are required.

## Building

Just like in the [Step 2 Building section](./step_2.md#building), the
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
[preamble of this tutorial](#tutorial-step-3-listening-and-reacting), this
example will use a virtual serial port created with the
[`socat` tool][socat_home]. Open a terminal that can remain open as a feedback
window, and run the following from the build tree:

```console
$ socat PTY,link=$PWD/pty,echo=0 STDIO
```

### Execution

Open another terminal, navigate to the build tree, and run the application as
follows:

```console
$ ./SerialCopyBack --serial-port=$PWD/pty
```

The application will not seem to do much on its own, but if switching to the
terminal window where `socat` is running and typing some text then pressing
"Enter" should send the typed text over the virtual serial port to the
example application. The application will report receipt with a log message, and
reply via the same serial port.

> [!WARNING]
> The example application as written is not very robust, and having the serial
> port "disappear" by closing down the `socat` session before the application
> will result in a flurry of error messages.

## Conclusion

Congratulations! 🎉🎉 You have successfully implemented two-way interaction with
an external entity via a serial port. That kind of full-duplex communication is
not far from how a production application would interact with a device.


[Cpp_Func_Objs]: https://en.cppreference.com/w/cpp/utility/functional "C++ Function Objects"
[Cpp_bind]: https://en.cppreference.com/w/cpp/utility/functional/bind "C++ std::bind()"
[Cpp_copy]: https://en.cppreference.com/w/cpp/algorithm/copy "C++ std::copy()"
[Cpp_mem_fn]: https://en.cppreference.com/w/cpp/utility/functional/mem_fn "C++ std::mem_fn()"
